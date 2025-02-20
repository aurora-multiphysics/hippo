#pragma once

#include "HippoPtr.h"
#include "fvCFD_moose.h"

#include <libmesh/point.h>

#include <map>
#include <unordered_map>

namespace Foam
{
template <typename T>
class IOList;
typedef int label;
typedef IOList<label> labelIOList;
} // namespace Foam

namespace Hippo
{

class FoamPoint
{
  double _pos[3];
  int32_t _gid = -1;

public:
  FoamPoint(double x0, double x1, double x2, int32_t id) : _pos{x0, x1, x2}, _gid(id) {};
  FoamPoint() {}

  libMesh::Point get_point() const { return libMesh::Point(_pos[0], _pos[1], _pos[2]); }

  int32_t get_id() const { return _gid; }

  double & operator[](int32_t i)
  {
    assert(i >= 0 && i < 3);
    return _pos[i];
  }
};

class FoamFace
{
  int32_t const * _start;
  int32_t const * _end;
  int32_t _subdomain_id = 0;
  int32_t _rank = 0;

public:
  FoamFace(int32_t const * start, int32_t const * end, int32_t subdomain_id = 0, int32_t rank = 0)
    : _start(start), _end(end), _subdomain_id(subdomain_id), _rank(rank)
  {
  }

  int32_t const & operator[](uint32_t const i) const
  {
    assert(_start + i < _end);
    return _start[i];
  }

  uint32_t size() const { return _end - _start; }
  int32_t const * begin() { return _start; }
  int32_t const * end() { return _end; }
  int32_t rank() const { return _rank; }
  int32_t subdomain_id() const { return _subdomain_id; }
};

class FvMeshWrapper
{
public:
  FvMeshWrapper(Foam::fvMesh * mesh) : _mesh(mesh) { calcGlobalData(); }

  int patchId(const std::string & name) const
  {
    auto id = _mesh->boundaryMesh().findIndex(name);
    assert(id != -1 && "Patch name does not exist");
    return id;
  }

  Foam::polyPatch const & patch(const int patch_id) const
  {
    return _mesh->boundaryMesh()[patch_id];
  }

  Foam::labelList const & uniquePoints() const { return _uniquePoints; }

  Foam::fvMesh & mesh() { return *_mesh; }

protected:
  void calcGlobalData()
  {
    _globalIndex = _mesh->globalData().mergePoints(_pointToGlobal, _uniquePoints);
  }

private:
  Foam::fvMesh * _mesh;
  Foam::autoPtr<Foam::globalIndex> _globalIndex;
  Foam::labelList _pointToGlobal;
  Foam::labelList _uniquePoints;
};

/**
 * Class to help convert an OpenFOAM mesh to a MOOSE one.
 */
class Foam2MooseMeshAdapter
{
public:
  Foam2MooseMeshAdapter(std::vector<std::string> patch_name,
                        Foam::fvMesh * fv_mesh,
                        MPI_Comm * comm = nullptr);
  ~Foam2MooseMeshAdapter();
  int32_t npoint();
  int32_t nface();
  FoamPoint const & point(uint32_t i);
  FoamFace face(uint32_t i);
  int get_patch_id(std::string const & patch_name);
  int get_gid(int32_t local, int32_t patch_id) const;
  int get_moose_id(int32_t global_id);

  size_t rank_element_offset{0};

private:
  FvMeshWrapper _mesh_wrapper;

  std::vector<std::string> _patch_name;
  // patch ids
  std::vector<int> _patch_id;
  // map from patch_id to a vector that maps local patch id to global id
  // need to keep these around to map from patch data to the moose mesh
  std::map<int, std::vector<int>> _patch_local2global;
  MPI_Comm * _comm;

  // Should contain every point (coord + global_id) from every rank
  HippoPtr<FoamPoint> _point;
  // flat array of points that make up each face e.g.
  // a mesh like this
  //  1 -- 2 -- 4
  //  | f0 | f1 |
  //  0 -- 3 -- 5
  //  would be for example  [0,1,2,3,3,2,4,5]
  HippoPtr<int32_t> _face_point_id;
  // index of begining of face n list of points from _face_point_id
  // so [0,4] in above example
  // so far only support quad elements so this is always
  // [0,4,8,12,...,4n,...]
  HippoPtr<int32_t> _face_offset;
  // subdomin of each face - TODO: this is != patch_id from openFOAM
  HippoPtr<int32_t> _face_subdom;
  // rank each face belongs to
  HippoPtr<int32_t> _face_rank;
  // local to global mapping - keep it around don't know what the perf is like
  // might be better to fix later
  std::unique_ptr<Foam::labelIOList> _loc2glob;

  std::unordered_map<int32_t, int32_t> _global2moose;

  void gather_unique_points(std::vector<FoamPoint> & unique_point);
  void gather_faces(std::vector<int32_t> & counts, std::vector<int32_t> & ids);

  void
  calc_subdom_and_rank_arrays(HippoPtr<int32_t> subdom_count, int32_t nsubdom, int32_t mpi_size);

  void set_up_serial();
  void set_up_parallel();
};
} // namespace Hippo

// Local Variables:
// mode: c++
// End:
