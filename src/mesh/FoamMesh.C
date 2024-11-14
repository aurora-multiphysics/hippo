#include "FoamMesh.h"
#include "FoamInterface.h"
#include "Foam2MooseMeshGen.h"

#include <memory>

registerMooseObject("hippoApp", FoamMesh);

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
    _foam_patch(getParam<std::vector<std::string>>("foam_patch")),
    _interface(Hippo::FoamInterface::getInstance({"-case", getParam<std::string>("case")},
                                                 _communicator.get()))
{
  auto comm = _communicator.get();
  int size = 1;
  MPI_Comm_size(comm, &size);
  _serial = (size == 1);
}

FoamMesh::FoamMesh(const FoamMesh & other_mesh)
  : MooseMesh(other_mesh), _foam_patch(other_mesh._foam_patch)
{
  auto comm = _communicator.get();
  int size = 1;
  MPI_Comm_size(comm, &size);
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
      _foam_patch, _interface, _serial ? nullptr : &comm);

  // TODO: Can reserve elements if _mesh->reserve_elements(#el)
  // TODO: Can also reserve nodes _mesh->reserve_nodes(#nodes?)
  for (int32_t pt = 0; pt < mesh_adapter->npoint(); ++pt)
  {
    auto foam_point = mesh_adapter->point(pt);
    _mesh->add_point(foam_point.get_point(), pt);
  }

  for (int32_t fc = 0; fc < mesh_adapter->nface(); ++fc)
  {
    auto face = mesh_adapter->face(fc);
    assert(face.size() == 4 && "Only support quads currently");
    auto elem = _mesh->add_elem(Elem::build_with_id(QUAD4, fc));

    int count = 0;
    for (auto point = face.begin(); point < face.end(); ++point)
    {
      auto moose_pt = mesh_adapter->get_moose_id(*point);
      elem->set_node(count++) = _mesh->node_ptr(moose_pt);
    }
    // TODO: need to look again at the subdomain_id so it matches the foam patch_id?
    elem->subdomain_id() = face.subdomain_id();
    elem->processor_id() = face.rank();
  }

  // patch id has the openfoam id that corresponds to the patch name
  // we set up the subdomains index to mirror the openfoam names
  for (auto const & patch_name : _foam_patch)
  {
    auto id = mesh_adapter->get_patch_id(patch_name);
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
// This needs to be here because of the incomplete type (MeshInterface) in the class
FoamMesh::~FoamMesh() { _interface->dropInstance(); };

libMesh::Elem *
FoamMesh::getElemPtr(int rank_local) const
{
  return _mesh->elem_ptr(rank_local);
}

// Local Variables:
// mode: c++
// End:
