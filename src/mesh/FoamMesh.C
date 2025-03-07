#include "FoamMesh.h"
#include "Foam2MooseMeshGen.h"

#include <IOobject.H>
#include <Pstream/mpi/PstreamGlobals.H>
#include <Time.H>
#include <argList.H>
#include <mpi.h>

#include <memory>

registerMooseObject("hippoApp", FoamMesh);

namespace
{
Foam::fvMesh
read_polymesh(const Foam::Time & run_time)
{
  Foam::IOobject mesh_header(
      Foam::fvMesh::defaultRegion, run_time.name(), run_time, Foam::IOobject::MUST_READ);
  return Foam::fvMesh(mesh_header);
}
}

InputParameters
FoamMesh::validParams()
{
  auto params = MooseMesh::validParams();
  params.addRequiredParam<std::vector<std::string>>("foam_patch",
                                                    "Name of foam boundary patches to replicate.");
  params.addRequiredParam<std::string>("case", "The directory containing the OpenFOAM case.");
  return params;
}

FoamMesh::FoamMesh(InputParameters const & params)
  : MooseMesh(params),
    _foam_patch(params.get<std::vector<std::string>>("foam_patch")),
    _foam_runtime(params.get<std::string>("case"), _communicator.get()),
    _foam_mesh(read_polymesh(_foam_runtime.runTime()))
{
  int size = 1;
  MPI_Comm_size(_communicator.get(), &size);
  _serial = (size == 1);
}

FoamMesh::FoamMesh(const FoamMesh & other_mesh)
  : MooseMesh(other_mesh),
    _foam_patch(other_mesh._foam_patch),
    _foam_runtime(other_mesh._foam_runtime),
    _foam_mesh(other_mesh._foam_mesh.clone())
{
  int size = 1;
  MPI_Comm_size(_communicator.get(), &size);
  _serial = (size == 1);
}

std::unique_ptr<MooseMesh>
FoamMesh::safeClone() const
{
  return std::make_unique<FoamMesh>(*this);
}

std::vector<int> &
FoamMesh::getSubdomainList()
{
  return _subdomain_list;
}

void
FoamMesh::buildMesh()
{
  auto comm = _communicator.get();
  auto mesh_adapter = std::make_unique<Hippo::Foam2MooseMeshAdapter>(
      _foam_patch, &_foam_mesh, _serial ? nullptr : &comm);

  // TODO: Can reserve elements if _mesh->reserve_elements(#el)
  // TODO: Can also reserve nodes _mesh->reserve_nodes(#nodes?)
  for (int32_t pt = 0; pt < mesh_adapter->npoint(); ++pt)
  {
    auto foam_point = mesh_adapter->point(pt);
    _mesh->add_point(foam_point.getPoint(), pt);
  }

  for (int32_t fc = 0; fc < mesh_adapter->nface(); ++fc)
  {
    auto face = mesh_adapter->face(fc);
    assert(face.size() == 4 && "Only support quads currently");
    auto elem = _mesh->add_elem(Elem::build_with_id(QUAD4, fc));

    int count = 0;
    for (auto point = face.begin(); point < face.end(); ++point)
    {
      auto moose_pt = mesh_adapter->getMooseId(*point);
      elem->set_node(count++) = _mesh->node_ptr(moose_pt);
    }
    // TODO: need to look again at the subdomain_id so it matches the foam
    // patch_id?
    elem->subdomain_id() = face.subdomain_id();
    elem->processor_id() = face.rank();
  }

  // patch id has the openfoam id that corresponds to the patch name
  // we set up the subdomains index to mirror the openfoam names
  for (auto const & patch_name : _foam_patch)
  {
    auto id = mesh_adapter->getPatchId(patch_name);
    this->setSubdomainName(id, patch_name);
    _subdomain_list.push_back(id);
  }

  rank_element_offset = mesh_adapter->rank_element_offset;

  // Need to be able to identify a moose node with a openFoam node
  _mesh->allow_renumbering(false);
  _mesh->skip_partitioning(true);
  _mesh->recalculate_n_partitions();
  libMesh::Partitioner::set_node_processor_ids(*_mesh);
  _mesh->prepare_for_use();
}

libMesh::Elem *
FoamMesh::getElemPtr(int rank_local) const
{
  return _mesh->elem_ptr(rank_local);
}
