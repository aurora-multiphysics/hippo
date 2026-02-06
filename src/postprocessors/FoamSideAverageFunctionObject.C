#include "FoamSideAverageFunctionObject.h"

registerMooseObject("hippoApp", FoamSideAverageFunctionObject);

InputParameters
FoamSideAverageFunctionObject::validParams()
{
  InputParameters params = FoamSideIntegratedFunctionObject::validParams();
  params.addClassDescription(
      "Class that averages a function object over OpenFOAM boundary patches.");
  return params;
}

void
FoamSideAverageFunctionObject::compute()
{
  _function_object->execute();
  _value = integrateValue(_function_object->name()) / getArea();
}
