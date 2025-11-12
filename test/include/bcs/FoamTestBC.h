#include "FoamBCBase.h"
#include "InputParameters.h"

class FoamTestBC : public FoamBCBase
{
public:
  explicit FoamTestBC(const InputParameters & params) : FoamBCBase(params) {};

  void imposeBoundaryCondition() {};
};
