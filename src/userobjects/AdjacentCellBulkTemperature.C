#include "AdjacentCellBulkTemperature.h"
#include "HippoBase.h"
#include "FoamProblem.h"
#include "MooseTypes.h"
#include "petsclog.h"
#include <Field.H>
#include <ListOps.H>
#include <Pstream/mpi/PstreamGlobals.H>
#include <PstreamReduceOps.H>
#include <UList.H>
#include <UPstream.H>
#include <VectorSpace.H>
#include <face.H>
#include <fvMesh.H>
#include <memory>
#include <ops.H>
#include <scalar.H>
#include <scalarField.H>
#include <vector>
#include <vectorField.H>
#include <KDTree.h>

registerMooseObject("hippoApp", AdjacentCellBulkTemperature);

InputParameters
AdjacentCellBulkTemperature::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<SubdomainName>("boundary", "Boundary this object applies to.");
  params.addParam<std::string>("T_name", "T", "Temperature name in the OpenFOAM simulation.");
  params.addParam<int>("max_leaf_size", 10, "Maximum leaf size to use for the KD tree search.");
  params.addParam<bool>(
      "reconstruct_tree", false, "Whether to rebuild the KD tree on each execution.");
  return params;
}

AdjacentCellBulkTemperature::AdjacentCellBulkTemperature(const InputParameters & params)
  : GeneralUserObject(params),
    HippoBase(this),
    _foam_patch{getFoamPatch(getParam<SubdomainName>("boundary"))},
    _kd_centres{},
    _gl_t_adjacent{},
    _kd_tree{}

{
}

void
AdjacentCellBulkTemperature::initialize()
{
  const int size = _foam_patch.size();
  int gl_size;

  const int nProcs{Foam::UPstream::nProcs()};
  const MPI_Comm comm{(nProcs == 1) ? MPI_COMM_WORLD : Foam::PstreamGlobals::MPI_COMM_FOAM};

  MPI_Allreduce(&size, &gl_size, 1, MPI_INT, MPI_SUM, comm);
  _gl_t_adjacent.resize(gl_size);
  _kd_centres.resize(gl_size);

  buildKDTree();
}

void
AdjacentCellBulkTemperature::buildKDTree()
{
  Foam::vectorField face_centres = _foam_patch.Cf();

  const int nProcs{Foam::UPstream::nProcs()};

  std::vector<int> face_sizes{nProcs};
  std::vector<int> face_displs{nProcs};
  _mpi_sizes.resize(nProcs);
  _mpi_displs.resize(nProcs);

  const MPI_Comm comm{(nProcs == 1) ? MPI_COMM_WORLD : Foam::PstreamGlobals::MPI_COMM_FOAM};

  // Create parameters for MPI gather operations to be reused in execute
  const int l_num_faces = face_centres.size();
  MPI_Allgather(&l_num_faces, 1, MPI_INT, _mpi_sizes.data(), 1, MPI_INT, comm);
  _mpi_displs[0] = 0;
  for (int i = 0; i < nProcs - 1; ++i)
  {
    _mpi_displs[i + 1] = _mpi_displs[i] + _mpi_sizes[i];
  }

  // Account for vector for gathering coordinates
  for (int i = 0; i < nProcs; ++i)
  {
    face_sizes[i] = _mpi_sizes[i] * 3;
    face_displs[i] = _mpi_displs[i] * 3;
  }

  Foam::vectorField gl_face_centres(_gl_t_adjacent.size());
  MPI_Allgatherv(face_centres.data(),
                 face_centres.size() * 3,
                 MPI_DOUBLE,
                 gl_face_centres.data(),
                 face_sizes.data(),
                 face_displs.data(),
                 MPI_DOUBLE,
                 comm);

  // Create KD tree
  for (int i = 0; i < gl_face_centres.size(); ++i)
  {
    const Foam::point & p = gl_face_centres[i];
    _kd_centres[i] = Point(p.x(), p.y(), p.z());
  }
  _kd_tree = std::make_unique<KDTree>(_kd_centres, getParam<int>("max_leaf_size"));
}

void
AdjacentCellBulkTemperature::execute()
{
  if (getParam<bool>("reconstruct_tree"))
    buildKDTree();

  // Gather Temperature on each execute
  Foam::scalarField l_t_adjacent{_foam_patch.Cf().size()};

  const Foam::fvMesh & foam_mesh{getFoamProblem().mesh().fvMesh()};
  const auto & T = foam_mesh.lookupObject<Foam::volScalarField>(getParam<std::string>("T_name"));
  for (int i = 0; i < l_t_adjacent.size(); ++i)
  {
    int idx = _foam_patch.faceCells()[i];
    l_t_adjacent[i] = T[idx];
  }

  const int nProcs{Foam::UPstream::nProcs()};
  const MPI_Comm comm{(nProcs == 1) ? MPI_COMM_WORLD : Foam::PstreamGlobals::MPI_COMM_FOAM};
  MPI_Allgatherv(l_t_adjacent.data(),
                 l_t_adjacent.size(),
                 MPI_DOUBLE,
                 _gl_t_adjacent.data(),
                 _mpi_sizes.data(),
                 _mpi_displs.data(),
                 MPI_DOUBLE,
                 comm);
}

const Foam::fvPatch &
AdjacentCellBulkTemperature::getFoamPatch(const std::string & boundary)
{
  const Foam::fvMesh & foam_mesh{getFoamProblem().mesh().fvMesh()};
  const Foam::label & boundaryId{foam_mesh.boundary().findIndex(boundary)};
  return foam_mesh.boundary()[boundaryId];
}

Real
AdjacentCellBulkTemperature::spatialValue(const Point & point) const
{
  const Foam::point p_moose{point(0), point(1), point(2)};
  std::vector<size_t> idx_vec;
  _kd_tree->neighborSearch(point, 1, idx_vec);
  return _gl_t_adjacent[idx_vec.at(0)];
}
