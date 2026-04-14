#include "FoamVariableBCBase.h"
#include "InputParameters.h"

class FoamTestBC : public FoamVariableBCBase
{
public:
  explicit FoamTestBC(const InputParameters & params) : FoamVariableBCBase(params) {};

  void imposeBoundaryCondition() {};
};
