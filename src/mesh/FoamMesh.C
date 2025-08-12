#include "FoamMesh.h"

#include "Foam2MooseMeshGen.h"
#include "libmesh/elem.h"
#include "libmesh/enum_elem_type.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_c0polygon.h"
#include "libmesh/face_tri3.h"
#include "libmesh/point.h"

#include <IOobject.H>
#include <Pstream/mpi/PstreamGlobals.H>
#include <Time.H>
#include <argList.H>
#include <mpi.h>

#include <memory>
#include "FoamDataStore.h"

registerMooseObject("hippoApp", FoamMesh);

namespace
{
static std::map<int, int> subdomain_id_map{
    {libMesh::TRI3, 1}, {libMesh::QUAD4, 2}, {libMesh::C0POLYGON, 3}};
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
    _foam_mesh(declareRecoverableData<Foam::fvMesh>("foam_mesh",
                                                    Foam::IOobject(Foam::fvMesh::defaultRegion,
                                                                   _foam_runtime.runTime().name(),
                                                                   _foam_runtime.runTime(),
                                                                   Foam::IOobject::MUST_READ)))
{
  int size = 1;
  MPI_Comm_size(_communicator.get(), &size);
  _serial = (size == 1);
}

FoamMesh::FoamMesh(const FoamMesh & other_mesh)
  : MooseMesh(other_mesh),
    _foam_patch(other_mesh._foam_patch),
    _foam_runtime(other_mesh._foam_runtime),
    _foam_mesh(other_mesh._foam_mesh)
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

inline bool
checkPointOnLine(const libMesh::Node * pt,
                 const libMesh::Node * start,
                 const libMesh::Node * end,
                 const double rtol = 1e-6)
{

  assert(pt != nullptr && pt != start && pt != end &&
         "checkPointOnLine: Point must not be nullptr or the same as start or end.");

  // Check if the point is on the line segment defined by start and end
  auto AP = *pt - *start;
  auto AB = *end - *start;

  // convert relative tolerance to absolute tolerance
  double const tol = AP.norm() * AB.norm() * rtol;

  auto cross_product = AB.cross(AP);
  if (cross_product.norm() > tol)
    return false; // Not collinear

  auto dot = AB * AP;
  if (dot < -tol)
    return false; // Point is before the start of the segment
  if (dot > AB.norm_sq() + tol)
    return false;

  return true;
}
std::unique_ptr<Elem>
FoamMesh::createElement(Hippo::Foam2MooseMeshAdapter * mesh_adapter, const Hippo::FoamFace & face)
{
  // get all MOOSE nodes associated with points in Foam face
  std::vector<libMesh::Node *> points;
  for (auto point = face.begin(); point < face.end(); ++point)
  {
    auto moose_pt = mesh_adapter->getMooseId(*point);
    points.push_back(_mesh->node_ptr(moose_pt));
  }

  // Remove collinear points on edge (due to more than one neighbouring
  // elements on this edge)

  // check last point
  while (checkPointOnLine(points.back(), points.end()[-2], points.front()))
    points.pop_back(); // remove last point if it is the same as the first

  // check in reverse the middle points
  // we start from the second last point and go to the second point
  for (auto point = points.end() - 2; point != points.begin(); --point)
  {
    if (checkPointOnLine(*point, *(point - 1), *(point + 1)))
      points.erase(point); // remove point if it is the same as the previous one
  }
  // check first point
  if (checkPointOnLine(points.front(), points.back(), points[1]))
    points.erase(points.begin()); // remove first point if it is the same as the last one

  // Create element based on number of points
  std::unique_ptr<libMesh::Elem> elem;
  if (points.size() == 3)
    elem = std::make_unique<libMesh::Tri3>();
  else if (points.size() == 4)
    elem = std::make_unique<libMesh::Quad4>();
  else
    elem = std::make_unique<libMesh::C0Polygon>(points.size());

  // set points, ranks and subdomain ids
  int count = 0;
  for (auto point : points)
  {
    elem->set_node(count++) = point;
  }
  elem->subdomain_id() = face.subdomain_id();
  elem->processor_id() = face.rank();

  return elem;
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
    _mesh->add_elem(createElement(mesh_adapter.get(), face));
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
