#pragma once

#include "FoamRuntime.h"
#include "Foam2MooseMeshGen.h"

#include <MooseTypes.h>
#include <MooseMesh.h>
#include <libmesh/elem.h>
#include <fixedGradientFvPatchField.H>
#include <fvPatchField.H>
#include <typeInfo.H>
#include <volFieldsFwd.H>
#include <argList.H>
#include <fvMesh.H>

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
  std::vector<SubdomainID> & getSubdomainList();
  bool isSerial() const { return _serial; }
  libMesh::Elem * getElemPtr(int local) const;
  Foam::fvMesh & fvMesh() { return _foam_mesh; }
  // Create a MOOSE element from a Foam face, removing collinear points on edges.
  std::unique_ptr<Elem> createElement(Hippo::Foam2MooseMeshAdapter * mesh_adapter,
                                      const Hippo::FoamFace & face);

  int64_t getPatchCount(int subdomain_id) const { return _patch_counts.at(subdomain_id); };
  int64_t getPatchOffset(int subdomain_id) const { return _patch_offsets.at(subdomain_id); };
  std::vector<int32_t> n_faces{0};
  // The index offset into the MOOSE element array, for the current rank.
  // This can be used with `getElemPtr` like so:
  //     getElemPtr(rank_offset + i)
  // to get the i-th element owned by the current rank from the mesh.
  size_t rank_element_offset{0};

  // Check if Foam mesh object has object of type T called name
  template <typename T>
  bool foamHasObject(Foam::word const & name)
  {
    return _foam_mesh.foundObject<T>(name);
  }

  // Returns the gradient BC array for field and subdomain
  template <typename GeoField, typename Type>
  Foam::Field<Type> & getGradientBCField(SubdomainID subdomain, Foam::word const & field)
  {
    auto & var = const_cast<Foam::fvPatchField<Type> &>(
        _foam_mesh.boundary()[subdomain].lookupPatchField<GeoField, Type>(field));
    return Foam::refCast<Foam::fixedGradientFvPatchField<Type>>(var).gradient();
  }

protected:
  std::vector<std::string> _foam_patch;
  Hippo::FoamRuntime _foam_runtime;
  Foam::fvMesh _foam_mesh;
  std::vector<int32_t> _patch_id;
  std::vector<SubdomainID> _subdomain_list;
  bool _serial = true;
  std::map<int, int64_t> _patch_counts;
  std::map<int, int64_t> _patch_offsets;
};
// Local Variables:
// mode: c++
// End:
