#pragma once

#include "FoamRuntime.h"
#include "libmesh/elem.h"
#include "Foam2MooseMeshGen.h"

#include <argList.H>
#include <fvMesh.H>
#include <MooseMesh.h>

#include <memory>

namespace Hippo
{
class Foam2MooseMeshAdapter;
}
namespace Foam
{
template <typename T>
class IOList;
typedef int label;
typedef IOList<label> labelIOList;
} // namespace Foam

class FoamMesh : public MooseMesh
{
public:
  static InputParameters validParams();
  FoamMesh(InputParameters const & params);
  FoamMesh(const FoamMesh & other_mesh);
  ~FoamMesh() = default;
  virtual std::unique_ptr<MooseMesh> safeClone() const override;
  virtual void buildMesh() override;
  std::vector<int> & getSubdomainList();
  bool isSerial() const { return _serial; }
  libMesh::Elem * getElemPtr(int local) const;
  Foam::fvMesh & fvMesh() { return _foam_mesh; }
  // Create a MOOSE element from a Foam face, removing collinear points on edges.
  std::unique_ptr<Elem> createElement(Hippo::Foam2MooseMeshAdapter * mesh_adapter,
                                      const Hippo::FoamFace & face);

  int64_t getPatchCount(int subdomainId) const { return _patch_counts[subdomainId]; };
  int64_t getPatchOffset(int subdomainId) const { return _patch_offsets[subdomainId]; };
  std::vector<int32_t> n_faces{0};
  // The index offset into the MOOSE element array, for the current rank.
  // This can be used with `getElemPtr` like so:
  //     getElemPtr(rank_offset + i)
  // to get the i-th element owned by the current rank from the mesh.
  size_t rank_element_offset{0};

protected:
  std::vector<std::string> _foam_patch;
  Hippo::FoamRuntime _foam_runtime;
  Foam::fvMesh _foam_mesh;
  std::vector<int32_t> _patch_id;
  std::vector<int> _subdomain_list;
  bool _serial = true;
  std::vector<int64_t> _patch_counts;
  std::vector<int64_t> _patch_offsets;
};
// Local Variables:
// mode: c++
// End:
