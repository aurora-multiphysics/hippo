#include "FoamVariableBCBase.h"
#include "InputParameters.h"

class FoamTestBC : public FoamVariableBCBase
{
public:
  static InputParameters validParams();
  explicit FoamTestBC(const InputParameters & params)
    : FoamVariableBCBase(params, FoamBCType::fixedValue) {};

  void imposeBoundaryCondition() {};
};
