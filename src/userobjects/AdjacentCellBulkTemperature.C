#include "AdjacentCellBulkTemperature.h"
#include "HippoBase.h"
#include "FoamProblem.h"
#include "MooseTypes.h"
#include "libmesh/libmesh_common.h"
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
#include <ops.H>
#include <scalar.H>
#include <scalarField.H>
#include <vector>
#include <vectorField.H>

registerMooseObject("hippoApp", AdjacentCellBulkTemperature);

InputParameters
AdjacentCellBulkTemperature::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<SubdomainName>("boundary", "Boundary this object applies to.");
  params.addParam<std::string>("T_name", "T", "Temperature name in the OpenFOAM simulation.");
  return params;
}

AdjacentCellBulkTemperature::AdjacentCellBulkTemperature(const InputParameters & params)
  : GeneralUserObject(params),
    HippoBase(this),
    _foam_patch{getFoamPatch(getParam<SubdomainName>("boundary"))},
    _gl_face_centres{},
    _gl_t_adjacent{}

{
}

void
AdjacentCellBulkTemperature::initialize()
{
  int size = _foam_patch.size();
  int gl_size;
  int nProcs{Foam::UPstream::nProcs()};
  MPI_Comm comm{(nProcs == 1) ? MPI_COMM_WORLD : Foam::PstreamGlobals::MPI_COMM_FOAM};

  MPI_Allreduce(&size, &gl_size, 1, MPI_INT, MPI_SUM, comm);
  _gl_face_centres.resize(gl_size, {-1, -1, -1});
  _gl_t_adjacent.resize(gl_size, -1);
}

void
AdjacentCellBulkTemperature::execute()
{
  Foam::vectorField face_centres = _foam_patch.Cf();
  int nProcs{Foam::UPstream::nProcs()};

  std::vector<int> sizes{nProcs};
  std::vector<int> displs{nProcs};
  std::vector<int> face_sizes{nProcs};
  std::vector<int> face_displs{nProcs};

  MPI_Comm comm{(nProcs == 1) ? MPI_COMM_WORLD : Foam::PstreamGlobals::MPI_COMM_FOAM};

  const int l_num_faces = face_centres.size();
  MPI_Allgather(&l_num_faces, 1, MPI_INT, sizes.data(), 1, MPI_INT, comm);
  displs[0] = 0;
  for (int i = 0; i < nProcs - 1; ++i)
  {
    displs[i + 1] = displs[i] + sizes[i];
  }

  for (int i = 0; i < nProcs; ++i)
  {
    face_sizes[i] = sizes[i] * 3;
    face_displs[i] = displs[i] * 3;
  }

  MPI_Allgatherv(face_centres.data(),
                 face_centres.size() * 3,
                 MPI_DOUBLE,
                 _gl_face_centres.data(),
                 face_sizes.data(),
                 face_displs.data(),
                 MPI_DOUBLE,
                 comm);

  Foam::scalarField l_t_adjacent{face_centres.size()};

  const Foam::fvMesh & foam_mesh{getFoamProblem().mesh().fvMesh()};
  const auto & T = foam_mesh.lookupObject<Foam::volScalarField>(getParam<std::string>("T_name"));
  for (int i = 0; i < l_t_adjacent.size(); ++i)
  {
    int idx = _foam_patch.faceCells()[i];
    l_t_adjacent[i] = T[idx];
  }

  MPI_Allgatherv(l_t_adjacent.data(),
                 l_t_adjacent.size(),
                 MPI_DOUBLE,
                 _gl_t_adjacent.data(),
                 sizes.data(),
                 displs.data(),
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
  Foam::label idx;
  Foam::scalar minDist = Foam::VGREAT;
  for (int i = 0; i < _gl_face_centres.size(); ++i)
  {
    const Foam::point & p = _gl_face_centres[i];
    const Foam::scalar dist{Foam::mag(p_moose - p)};
    if (dist < minDist)
    {
      idx = i;
      minDist = dist;
    }
  }
  return _gl_t_adjacent[idx];
}
