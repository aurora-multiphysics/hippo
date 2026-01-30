#include "FoamSideAverageFunctionObject.h"

registerMooseObject("hippoApp", FoamSideAverageFunctionObject);

void
FoamSideAverageFunctionObject::compute()
{
  _function_object->execute();
  _value = integrateValue(_function_object->name()) / getArea();
}
