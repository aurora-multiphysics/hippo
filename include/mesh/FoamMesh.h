#pragma once
#include <memory>
#include "MooseMesh.h"
#include "FoamInterface.h"

namespace Hippo
{
class MeshInterface;
}
namespace Foam
{
template <typename T>
class IOList;
typedef long int label;
typedef IOList<label> labelIOList;
}

class FoamMesh : public MooseMesh
{
public:
  static InputParameters validParams();
  FoamMesh(InputParameters const & params);
  FoamMesh(const FoamMesh & other_mesh);
  ~FoamMesh(); //{ _interface->dropInstance(); };
  virtual std::unique_ptr<MooseMesh> safeClone() const override;
  virtual void buildMesh() override;
  Hippo::FoamInterface * getFoamInterface() { return _interface; }
  std::vector<int> & getSubdomainList();
  bool isSerial() const { return _serial; }
  int getGid(int local, int patch_id) const;
  libMesh::Node * getNodePtr(int local, int patch_id) const;

protected:
  std::vector<std::string> _foam_patch;
  std::vector<int32_t> _patch_id;
  Hippo::FoamInterface * _interface;
  std::vector<int> _subdomain_list;
  std::unique_ptr<Hippo::MeshInterface> _fmesh;
  bool _serial = true;
};
// Local Variables:
// mode: c++
// End:
