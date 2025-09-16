#include "InputParameters.h"
#include "MooseObject.h"

class FoamBCBase : public MooseObject
{
public:
  static InputParameters validParams();

  explicit FoamBCBase(const InputParameters & params);
};
