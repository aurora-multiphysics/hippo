#include "FoamInterface.h"
#include "Foam2MooseMeshGen.h"
#include "fvCFD_moose.h"
#include "PrimitivePatch.H"
#include <numeric>
#include <memory>
#include <algorithm>
#include <set>
#include <unordered_map>
#include <vector>
#include <MooseError.h>
#include "HippoPtr.h"
#include "CommUtil.h"
namespace Hippo
{

// If we are running serial pass this in as the mesh_to_global_map
// to do nothing but preserve the function behaviour
struct IdMap
{
  int32_t operator[](int32_t const id) const { return id; }
};

template <typename T>
static void
append_local_points(T const & mesh_to_global_map,
                    std::set<Foam::label> & unique_point,
                    Foam::polyPatch const & poly_patch,
                    std::vector<FoamPoint> & local_point)

{
  // auto unique_point = interface->uniquePoints();
  auto mesh_point_map = poly_patch.meshPointMap();
  auto point = poly_patch.localPoints();
  // Maybe better to count the points first then
  // reserve but probably fine
  for (auto it = unique_point.begin(); it != unique_point.end();)
  {
    auto local_id = mesh_point_map.find(*it);
    if (local_id != mesh_point_map.end())
    {
      local_point.emplace_back(point[*local_id][0],
                               point[*local_id][1],
                               point[*local_id][2],
                               static_cast<int32_t>(mesh_to_global_map[*it]));
      it = unique_point.erase(it);
    }
    else
      it++;
  }
}

template <typename T>
std::vector<FoamPoint>
get_local_points(T const & mesh_to_global_map,
                 FoamInterface * interface,
                 std::vector<std::string> & patch_list)
{
  auto unique_point = interface->uniquePoints();
  std::vector<FoamPoint> local_point;
  std::set<Foam::label> unique_point_set;
  // Create a set to hold local points
  // append_function removes them as they are added so we don't double
  // count. Must be a nicer way but this is simple enough for now
  for (auto const pt : unique_point)
    unique_point_set.insert(pt);

  for (auto const & patch_name : patch_list)
  {
    append_local_points<T>(
        mesh_to_global_map, unique_point_set, interface->getPatch(patch_name), local_point);
  }

  return local_point;
}

template <typename T>
std::vector<int>
create_local2global_map(T const & mesh_to_global_map, Foam::polyPatch const & poly_patch)
{
  auto mesh_point = poly_patch.meshPoints();
  auto mesh_point_map = poly_patch.meshPointMap();
  // TODO: Not sure if meshPoints is ordered i.e. mesh_point[0] is local_point 0
  // the comment in PrimitivePatch.H suggests it's constructed walking the
  // faces - so don't know what that means
  // So going around the houses a bit

  // create a map from local(patch) id to global (across processes) ids
  //
  std::vector<int32_t> patch_to_global(mesh_point.size(), -1);
  for (auto const pt : mesh_point)
  {
    auto lid = mesh_point_map[pt];
    patch_to_global[lid] = mesh_to_global_map[pt];
  }
  return patch_to_global;
}
int32_t
append_local_faces(std::vector<int> patch_to_global,
                   Foam::List<Foam::face> const & local_face,
                   std::vector<int32_t> & local_face_offset,
                   std::vector<int32_t> & local_face_point_id)
{
  // auto local_face = poly_patch.localFaces();
  auto nfaces = local_face.size();
  local_face_offset.reserve(nfaces + 1);
  // TODO: should check all faces aren't the same number of
  // points would make life easier
  for (auto const & face : local_face)
  {
    local_face_offset.push_back(face.size());
  }

  for (auto const & face : local_face)
  {
    for (int i = 0; i < face.size(); ++i)
    {
      auto glob_id = patch_to_global[face[i]];
      local_face_point_id.push_back(glob_id);
    }
  }
  return nfaces;
}

struct PatchInfo
{
  std::vector<int32_t> count;
  std::vector<int32_t> point_id;
  std::vector<int32_t> subdomain_count;
  // The number of elements the current rank has in its domain.
  // These do not necessarily belong to the same patch.
  size_t rank_element_count{0};
  std::map<int32_t, std::vector<int32_t>> local2global;
};

template <typename T>
PatchInfo
get_local_face_info(T const & mesh_to_global_map,
                    FoamInterface * interface,
                    std::vector<int> & patch_id)
{
  PatchInfo patch_info;
  for (auto const & id : patch_id)
  {
    auto patch = interface->getPatch(id);

    auto local2global = create_local2global_map<T>(mesh_to_global_map, patch);
    // patch_info.local2global[id] = create_local2global_map<T>(mesh_to_global_map, patch);
    auto const & local_face = patch.localFaces();
    auto nfaces =
        append_local_faces(local2global, local_face, patch_info.count, patch_info.point_id);
    patch_info.subdomain_count.push_back(nfaces);
    patch_info.rank_element_count += nfaces;
    // TODO: need to check this is right
    patch_info.local2global[id] = std::move(local2global);
  }
  return patch_info;
}

std::unique_ptr<Foam::labelIOList>
get_local_global_map(FoamInterface * interface)
{
  auto & mesh = interface->getMesh();
  Foam::typeIOobject<Foam::labelIOList> addrHeader("pointProcAddressing",
                                                   mesh.facesInstance() / mesh.meshSubDir,
                                                   mesh,
                                                   Foam::IOobject::MUST_READ);
  if (!addrHeader.headerOk())
  {
    mooseError("Failed to read pointProcAddressing\n");
  }

  return std::make_unique<Foam::labelIOList>(addrHeader);
}

void
MeshInterface::gather_faces(std::vector<int32_t> & local_face_count,
                            std::vector<int32_t> & local_face_point_id)
{
  _face_point_id = gather_vector(local_face_point_id, *_comm);
  _face_offset = gather_and_scan_vector<int32_t>(local_face_count, *_comm);
}

void
MeshInterface::gather_unique_points(std::vector<FoamPoint> & local_point)
{
  _point = gather_vector<FoamPoint>(local_point, *_comm);
}

// local_subd_count has the number of faces for each subdomain
//  -> the total is the number of faces on each subdomain
//  -> we can use that to create the rank array too

void
MeshInterface::calc_subdom_and_rank_arrays(HippoPtr<int32_t> subdom_count,
                                           int32_t nsubdom,
                                           int32_t mpi_size)
{
  auto nface = subdom_count.back();
  _face_subdom = HippoPtr<int32_t>(nface);
  _face_rank = HippoPtr<int32_t>(nface);
  int count = 0;
  for (int rank = 0; rank < mpi_size; ++rank)
  {
    for (int subdom = 0; subdom < nsubdom; ++subdom)
    {
      for (int id = subdom_count[rank * nsubdom + subdom];
           id < subdom_count[rank * nsubdom + subdom + 1];
           ++id)
      {
        _face_subdom[count] = subdom;
        _face_rank[count] = rank;
        // check count < number of faces
        assert(count < nface);
        ++count;
      }
    }
  }
}

void
MeshInterface::set_up_serial()
{
  // This is suboptimal (computationally and codeistically)
  // but serial is not the common case so havn't optimised
  _interface->calcGlobalData();
  auto local_point = get_local_points<IdMap>(IdMap(), _interface, _patch_name);
  std::vector<int32_t> face_count, face_point_id, face_subdomain_id;
  auto face_info = get_local_face_info<IdMap>(IdMap(), _interface, _patch_id);

  _point = copy_vec_to_pointer(local_point);
  _face_offset = scan_vec_to_pointer(face_info.count);
  _face_point_id = copy_vec_to_pointer(face_info.point_id);
  auto subdom_count = scan_vec_to_pointer(face_info.subdomain_count);
  // Save the local2global map for each patch, only need this locally (for now)
  // so don't need prep for MPI or anythin
  assert(subdom_count.size() - 1 == _patch_name.size());
  assert(subdom_count.back() == this->nface());
  // TODO: check this is doing the right thing
  _patch_local2global = std::move(face_info.local2global);

  calc_subdom_and_rank_arrays(std::move(subdom_count), _patch_name.size(), 1);
}

void
MeshInterface::set_up_parallel()
{
  _interface->calcGlobalData();
  _loc2glob = get_local_global_map(_interface);
  auto local_point = get_local_points<Foam::labelIOList>(*_loc2glob, _interface, _patch_name);
  std::vector<int32_t> face_count, face_point_id, face_subdomain_id;
  auto face_info = get_local_face_info<Foam::labelIOList>(*_loc2glob, _interface, _patch_id);
  gather_unique_points(local_point);
  gather_faces(face_info.count, face_info.point_id);

  // Gather the indices of the start of each patch in each rank.
  // Concatenate them and scan.
  auto global_subdom_count = gather_and_scan_vector<int32_t>(face_info.subdomain_count, *_comm);

  int mpi_size;
  MPI_Comm_size(*_comm, &mpi_size);
  assert(global_subdom_count.size() - 1 == _patch_name.size() * mpi_size);
  assert(global_subdom_count.back() == this->nface());
  calc_subdom_and_rank_arrays(std::move(global_subdom_count), _patch_name.size(), mpi_size);

  // Gather the indices of the start of the element index for each rank.
  // Note that this will be the same as 'global_subdom_count' if there is only
  // one patch in each rank's decomposition domain.
  rank_element_offset = mpi_scan<size_t>(face_info.rank_element_count, *_comm);

  _patch_local2global = std::move(face_info.local2global);
}

MeshInterface::MeshInterface(std::vector<std::string> const & patch_name,
                             FoamInterface * foam_interface,
                             MPI_Comm * comm)
  : _patch_name(patch_name), _interface(foam_interface), _comm(comm)
{
  _patch_id.reserve(_patch_name.size());
  for (auto const & name : patch_name)
  {
    _patch_id.push_back(get_patch_id(name));
  }

  if (comm)
    this->set_up_parallel();
  else
    this->set_up_serial();

  // TODO: Move this - this is some annoying bookeeping I need to change the
  // patch ids in the local2global maps so that they point to the id in the moose mesh
  // I can't find a way to stop moose renumbering the nodes so I can't
  // use the openFoam global ids
  int count = 0;
  for (auto pt : _point)
  {
    _global2moose[pt.get_id()] = count++;
  }
  for (auto & map : _patch_local2global)
  {
    for (auto v = map.second.begin(); v != map.second.end(); ++v)
    {
      auto moose_id = _global2moose[*v];
      *v = moose_id;
    }
  }
}

MeshInterface::~MeshInterface() = default;

int32_t
MeshInterface::npoint()
{
  return _point.size();
}

int32_t
MeshInterface::nface()
{
  return _face_offset.size() - 1;
}

FoamPoint const &
MeshInterface::point(int32_t i)
{
  assert(i >= 0 && i < _point.size());
  return _point[i];
}

FoamFace
MeshInterface::face(int32_t i)
{
  assert(i >= 0 && i < _face_offset.size());
  return FoamFace(_face_point_id.begin() + _face_offset[i],
                  _face_point_id.begin() + _face_offset[i + 1],
                  _face_subdom[i],
                  _face_rank[i]);
}

int
MeshInterface::get_patch_id(std::string const & name)
{
  return _interface->getPatchID(name);
}

int
MeshInterface::get_gid(int32_t local, int32_t patch_id) const
{
  auto const & patch_map = _patch_local2global.find(patch_id);
  assert(patch_map != _patch_local2global.end());
  return patch_map->second[local];
}

int
MeshInterface::get_moose_id(int32_t global_id)
{
  return _global2moose[global_id];
}
}
