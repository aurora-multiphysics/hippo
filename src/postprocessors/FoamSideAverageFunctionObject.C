#include "FoamSideAverageFunctionObject.h"

registerMooseObject("hippoApp", FoamSideAverageFunctionObject);

void
FoamSideAverageFunctionObject::compute()
{
  _value = integrateValue() / getArea();
}
