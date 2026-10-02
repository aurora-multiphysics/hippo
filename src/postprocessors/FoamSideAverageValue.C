#include "FoamSideAverageValue.h"
#include "InputParameters.h"

registerMooseObject("hippoApp", FoamSideAverageValue);

InputParameters
FoamSideAverageValue::validParams()
{
  InputParameters params = FoamSideIntegratedValue::validParams();
  params.addClassDescription(
      "Computes the area-weighted average of an OpenFOAM scalar field or selected vector "
      "component over the selected boundary patches.");
  return params;
}

FoamSideAverageValue::FoamSideAverageValue(const InputParameters & params)
  : FoamSideIntegratedValue(params)
{
}

void
FoamSideAverageValue::compute()
{
  _value = integrateValue(_foam_variable) / getArea();
}
