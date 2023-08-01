#include <memory>
#include "FoamMesh.h"
#include "FoamInterface.h"
#include "Foam2MooseMeshGen.h"
#include "libmesh/face_quad4.h"

registerMooseObject("hippoApp", FoamMesh);

InputParameters
FoamMesh::validParams()
{
  auto params = MooseMesh::validParams();
  // make a vector at some point
  params.addRequiredParam<std::vector<std::string>>("foam_patch",
                                                    "Name of foam boundary patches to replicate");

  std::vector<std::string> empty_vec;
  params.addParam<std::vector<std::string>>(
      "foam_args", empty_vec, "List of arguments to be passed to openFoam solver");
  return params;
}

FoamMesh::FoamMesh(InputParameters const & params)
  : MooseMesh(params),
    _foam_patch(getParam<std::vector<std::string>>("foam_patch")),
    _interface(Hippo::FoamInterface::getInstance(getParam<std::vector<std::string>>("foam_args"),
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
  _fmesh =
      std::make_unique<Hippo::MeshInterface>(_foam_patch, _interface, (_serial) ? nullptr : &comm);

  // TODO: Can reserve elements if _mesh->reserve_elements(#el)
  // TODO: Can also reserve nodes _mesh->reserve_nodes(#nodes?)
  for (int32_t pt = 0; pt < _fmesh->npoint(); ++pt)
  {
    auto foam_point = _fmesh->point(pt);
    _mesh->add_point(foam_point.get_point(), pt);
  }

  for (int32_t fc = 0; fc < _fmesh->nface(); ++fc)
  {
    auto face = _fmesh->face(fc);
    assert(face.size() == 4 && "Only support quads currently");
    auto elem = _mesh->add_elem(Elem::build_with_id(QUAD4, fc));

    int count = 0;
    for (auto point = face.begin(); point < face.end(); ++point)
    {
      auto moose_pt = _fmesh->get_moose_id(*point);
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
    auto id = _fmesh->get_patch_id(patch_name);
    this->setSubdomainName(id, patch_name);
    _subdomain_list.push_back(id);
  }

  // Need to be able to identify a moose node with a openFoam node
  _mesh->allow_renumbering(false);
  //_mesh->skip_partitioning(true);
  //_mesh->recalculate_n_partitions();
  // libMesh::Partitionaer::set_node_processor_ids(*_mesh);
  _mesh->prepare_for_use();
}
// This needs to be here because of the incomplete type (MeshInterface) in the class
FoamMesh::~FoamMesh() { _interface->dropInstance(); };

int
FoamMesh::getGid(int local, int patch_id) const
{
  return _fmesh->get_gid(local, patch_id);
}

libMesh::Node *
FoamMesh::getNodePtr(int local, int patch_id) const
{
  auto gid = _fmesh->get_gid(local, patch_id);
  return _mesh->node_ptr(gid);
}

// Local Variables:
// mode: c++
// End:
