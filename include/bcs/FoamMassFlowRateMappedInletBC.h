#include "FoamBCBase.h"
#include <utility>

class FoamMassFlowRateMappedInletBC : public FoamBCBase, public PostprocessorInterface
{
public:
  static InputParameters validParams();

  FoamMassFlowRateMappedInletBC(const InputParameters & params);

  virtual void imposeBoundaryCondition() override;

  virtual void initialSetup() override;

protected:
  PostprocessorName _pp_name;

  std::vector<Real> _offset;

  std::map<int, std::vector<int>> _send_map;

  std::map<int, std::vector<int>> _recv_map;

  void createPatchProcMap();

  template <typename T>
  Foam::Field<T> getMappedArray(const Foam::word & name);
};
