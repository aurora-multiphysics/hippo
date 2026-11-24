#include "FoamVariableBCBase.h"
#include "InputParameters.h"

class FoamTestBC : public FoamVariableBCBase
{
public:
  static InputParameters validParams();
  explicit FoamTestBC(const InputParameters & params) : FoamVariableBCBase(params) {};

  void imposeBoundaryCondition(bool initialisation = false) override {};

protected:
  void constructFoamPatch(Foam::label patch_id) override;
};
