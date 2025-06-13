#include "CommUtil.h"
#include "Foam2MooseMeshGen.h"
#include "HippoPtr.h"
#include "fvMesh.H"

#include <MooseError.h>

#include <algorithm>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

namespace Hippo
{

// If we are running serial pass this in as the mesh_to_global_map
// to do nothing but preserve the function behaviour
struct IdMap
{
  int32_t operator[](int32_t const id) const { return id; }
  int32_t * find(int32_t & id) const { return &id; }
  int32_t size() const { return 0; }
};

template <typename T>
static void
appendLocalPoints(T const & mesh_to_global_map,
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
getLocalPoints(T const & mesh_to_global_map,
               std::vector<int> const & patch_ids,
               FvMeshWrapper const & mesh_wrapper)
{
  std::vector<FoamPoint> local_point;
  std::set<Foam::label> unique_point_set;
  // Create a set to hold local points
  // append_function removes them as they are added so we don't double
  // count. Must be a nicer way but this is simple enough for now
  for (auto const pt : mesh_wrapper.uniquePoints())
    unique_point_set.insert(pt);

  for (auto const & patch_id : patch_ids)
  {
    auto patch = mesh_wrapper.patch(patch_id);
    appendLocalPoints<T>(mesh_to_global_map, unique_point_set, patch, local_point);
  }

  return local_point;
}

template <typename T>
std::vector<int>
createLocal2GlobalMap(T const & mesh_to_global_map, Foam::polyPatch const & poly_patch)
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
appendLocalFaces(std::vector<int> patch_to_global,
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

void
appendMissingPoints(HippoPtr<FoamPoint> & point,
                    Foam::labelIOList const & mesh_to_global_map,
                    std::set<Foam::label> & unique_point,
                    Hippo::FvMeshWrapper const & mesh_wrapper,
                    Foam::polyPatch const & poly_patch,
                    MPI_Comm * _comm)
{
  auto mesh_point_map = poly_patch.meshPointMap();
  auto local_point = mesh_wrapper.points();

  std::vector<int> global_ids(point.size());
  std::transform(
      point.begin(), point.end(), global_ids.begin(), [](auto & pt) { return pt.getId(); });

  std::vector<int> local_patch_points;
  for (auto const & pt : mesh_point_map.toc())
  {
    local_patch_points.push_back(pt);
  }

  std::map<int, int> global_to_local_map;
  for (int i = 0; i < local_point.size(); ++i)
  {
    global_to_local_map[mesh_to_global_map[i]] = i;
  }

  std::vector<int> missing_ids;
  for (auto const & pt : local_patch_points)
  {
    // Check if the point is in the mesh
    auto it = std::find(global_ids.begin(), global_ids.end(), mesh_to_global_map[pt]);
    if (it == global_ids.end())
    {
      // This point is not in the mesh
      missing_ids.push_back(mesh_to_global_map[pt]);
    }
  }

  auto global_missing = gather_vector<int>(missing_ids, *_comm);
  if (global_missing.size() > 0)
  {
    std::vector<int> global_unique(unique_point.size());
    std::transform(unique_point.begin(),
                   unique_point.end(),
                   global_unique.begin(),
                   [&mesh_to_global_map](auto & pt) { return mesh_to_global_map[pt]; });

    std::vector<FoamPoint> missing_point;
    for (auto const & id : global_missing)
    {
      auto it = std::find(global_unique.begin(), global_unique.end(), id);
      if (it != global_unique.end())
      {
        auto pt = local_point[global_to_local_map[*it]];
        missing_point.emplace_back(pt[0], pt[1], pt[2], id);
      }
    }

    auto point_vec = point.to_std_vector();
    auto global_missing_points = gather_vector<FoamPoint>(missing_point, *_comm);
    point_vec.insert(point_vec.end(), global_missing_points.begin(), global_missing_points.end());
    point = HippoPtr<FoamPoint>(point_vec);
  }
}

void
getMissingPoints(HippoPtr<FoamPoint> & point,
                 Foam::labelIOList const & mesh_to_global_map,
                 std::vector<int> const & patch_ids,
                 FvMeshWrapper const & mesh_wrapper,
                 MPI_Comm * _comm)
{
  std::vector<FoamPoint> local_point;
  std::set<Foam::label> unique_point_set;
  // Create a set to hold local points
  // append_function removes them as they are added so we don't double
  // count. Must be a nicer way but this is simple enough for now
  for (auto const pt : mesh_wrapper.uniquePoints())
    unique_point_set.insert(pt);

  for (auto const & patch_id : patch_ids)
  {
    auto patch = mesh_wrapper.patch(patch_id);
    appendMissingPoints(point, mesh_to_global_map, unique_point_set, mesh_wrapper, patch, _comm);
  }
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
getLocalFaceInfo(T const & mesh_to_global_map,
                 const FvMeshWrapper & mesh_wrapper,
                 std::vector<int> & patch_id)
{
  PatchInfo patch_info;
  for (auto const & id : patch_id)
  {
    auto patch = mesh_wrapper.patch(id);

    auto local2global = createLocal2GlobalMap<T>(mesh_to_global_map, patch);
    // patch_info.local2global[id] =
    // create_local2global_map<T>(mesh_to_global_map, patch);
    auto const & local_face = patch.localFaces();
    auto nfaces = appendLocalFaces(local2global, local_face, patch_info.count, patch_info.point_id);
    patch_info.subdomain_count.push_back(nfaces);
    patch_info.rank_element_count += nfaces;
    // TODO: need to check this is right
    patch_info.local2global[id] = std::move(local2global);
  }
  return patch_info;
}

std::unique_ptr<Foam::labelIOList>
getLocalGlobalMap(const Foam::fvMesh & mesh)
{
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
Foam2MooseMeshAdapter::gatherFaces(std::vector<int32_t> & local_face_count,
                                   std::vector<int32_t> & local_face_point_id)
{
  _face_point_id = gather_vector(local_face_point_id, *_comm);
  _face_offset = gather_and_scan_vector<int32_t>(local_face_count, *_comm);
}

void
Foam2MooseMeshAdapter::gatherUniquePoints(std::vector<FoamPoint> & local_point)
{
  _point = gather_vector<FoamPoint>(local_point, *_comm);
}

// local_subd_count has the number of faces for each subdomain
//  -> the total is the number of faces on each subdomain
//  -> we can use that to create the rank array too

void
Foam2MooseMeshAdapter::calcSubdomAndRankArrays(HippoPtr<int32_t> subdom_count,
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
Foam2MooseMeshAdapter::setUpSerial()
{
  // This is suboptimal (computationally and codeistically)
  // but serial is not the common case so havn't optimised
  auto local_point = getLocalPoints<IdMap>(IdMap(), _patch_id, _mesh_wrapper);
  std::vector<int32_t> face_count, face_point_id, face_subdomain_id;
  auto face_info = getLocalFaceInfo<IdMap>(IdMap(), _mesh_wrapper, _patch_id);

  _point = copy_vec_to_pointer(local_point);
  _face_offset = scan_vec_to_pointer(face_info.count);
  _face_point_id = copy_vec_to_pointer(face_info.point_id);
  auto subdom_count = scan_vec_to_pointer(face_info.subdomain_count);
  assert(subdom_count.back() == this->nface());
  // TODO: check this is doing the right thing
  _patch_local2global = std::move(face_info.local2global);

  calcSubdomAndRankArrays(std::move(subdom_count), _patch_name.size(), 1);
}

void
Foam2MooseMeshAdapter::setUpParallel()
{
  // _loc2glob should be a map from the local id to the global id and it is read directly from the
  // pointProcAddressing file
  _loc2glob = getLocalGlobalMap(_mesh_wrapper.mesh());
  auto local_point = getLocalPoints<Foam::labelIOList>(*_loc2glob, _patch_id, _mesh_wrapper);
  std::vector<int32_t> face_count, face_point_id, face_subdomain_id;
  auto face_info = getLocalFaceInfo<Foam::labelIOList>(*_loc2glob, _mesh_wrapper, _patch_id);
  gatherUniquePoints(local_point);

  // This resolves an edge case
  // - getLocalPoints gets all the unique_points on the relevant interface boundary patches
  // - therefore, a face on a rank may need a point from the adjacent rank
  // - but that point on the adjacent rank may not be associated with a face on the interface so
  //   that point will not be in that rank's local points
  // - so this functions gets each rank to ensure all the points required are present and if not
  // request them be collected It may be useful to refactor how the points are gathered in the
  // future
  getMissingPoints(_point, *_loc2glob, _patch_id, _mesh_wrapper, _comm);

  gatherFaces(face_info.count, face_info.point_id);

  // Gather the indices of the start of each patch in each rank.
  // Concatenate them and scan.
  auto global_subdom_count = gather_and_scan_vector<int32_t>(face_info.subdomain_count, *_comm);

  int mpi_size;
  MPI_Comm_size(*_comm, &mpi_size);
  assert(global_subdom_count.size() - 1 == _patch_name.size() * static_cast<size_t>(mpi_size));
  assert(global_subdom_count.back() == this->nface());
  calcSubdomAndRankArrays(std::move(global_subdom_count), _patch_name.size(), mpi_size);

  // Gather the indices of the start of the element index for each rank.
  // Note that this will be the same as 'global_subdom_count' if there is only
  // one patch in each rank's decomposition domain.
  rank_element_offset = mpi_scan<size_t>(face_info.rank_element_count, *_comm);

  _patch_local2global = std::move(face_info.local2global);
}

Foam2MooseMeshAdapter::Foam2MooseMeshAdapter(std::vector<std::string> patch_name,
                                             Foam::fvMesh * fv_mesh,
                                             MPI_Comm * comm)
  : _mesh_wrapper(fv_mesh), _patch_name(std::move(patch_name)), _comm(comm)
{
  std::transform(_patch_name.begin(),
                 _patch_name.end(),
                 std::back_inserter(_patch_id),
                 [this](const auto & name) { return this->getPatchId(name); });

  if (comm)
  {
    this->setUpParallel();
  }
  else
  {
    this->setUpSerial();
  }

  // TODO: Move this - this is some annoying bookeeping I need to change the
  // patch ids in the local2global maps so that they point to the id in the
  // moose mesh I can't find a way to stop moose renumbering the nodes so I
  // can't use the openFoam global ids
  int count = 0;
  for (auto pt : _point)
  {
    _global2moose[pt.getId()] = count++;
  }
  for (auto & map : _patch_local2global)
  {
    for (auto v = map.second.begin(); v != map.second.end(); ++v)
    {
      auto kv = _global2moose.find(*v);
      if (kv == _global2moose.end())
      {
        mooseError("getMooseId failed to find global id, ", *v, " in the moose mesh\n");
      }
      auto moose_id = _global2moose[*v];
      *v = moose_id;
    }
  }
}

Foam2MooseMeshAdapter::~Foam2MooseMeshAdapter() = default;

int32_t
Foam2MooseMeshAdapter::npoint()
{
  return _point.size();
}

int32_t
Foam2MooseMeshAdapter::nface()
{
  return _face_offset.size() - 1;
}

FoamPoint const &
Foam2MooseMeshAdapter::point(uint32_t i)
{
  assert(i < _point.size());
  return _point[i];
}

FoamFace
Foam2MooseMeshAdapter::face(uint32_t i)
{
  assert(i < _face_offset.size());
  return FoamFace(_face_point_id.begin() + _face_offset[i],
                  _face_point_id.begin() + _face_offset[i + 1],
                  _face_subdom[i],
                  _face_rank[i]);
}

int
Foam2MooseMeshAdapter::getPatchId(std::string const & name)
{
  return _mesh_wrapper.patchId(name);
}

int
Foam2MooseMeshAdapter::getGid(int32_t local, int32_t patch_id) const
{
  auto const & patch_map = _patch_local2global.find(patch_id);
  assert(patch_map != _patch_local2global.end());
  return patch_map->second[local];
}

int
Foam2MooseMeshAdapter::getMooseId(int32_t global_id)
{
  auto kv = _global2moose.find(global_id);
  if (kv == _global2moose.end())
    mooseError("getMooseId failed to find global id, ", global_id, " in the moose mesh\n");

  return kv->second;
}
} // namespace Hippo
