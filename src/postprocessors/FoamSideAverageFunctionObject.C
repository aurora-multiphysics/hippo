#include "FoamSideAverageFunctionObject.h"

registerMooseObject("hippoApp", FoamSideAverageFunctionObject);

InputParameters
FoamSideAverageFunctionObject::validParams()
{
  InputParameters params = FoamSideIntegratedFunctionObject::validParams();
  params.addClassDescription(
      "Computes the area-weighted average of the field produced by an OpenFOAM function object "
      "over the selected boundary patches, using the selected component for vector fields.");
  return params;
}

void
FoamSideAverageFunctionObject::compute()
{
  _function_object->execute();
  _value = integrateValue(_function_object->name()) / getArea();
}
