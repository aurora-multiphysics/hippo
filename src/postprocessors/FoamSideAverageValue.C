#include "FoamSideAverageValue.h"
#include "InputParameters.h"

registerMooseObject("hippoApp", FoamSideAverageValue);

InputParameters
FoamSideAverageValue::validParams()
{
  InputParameters params = FoamSideIntegratedValue::validParams();
  params.addClassDescription("Class that averages a variable over OpenFOAM boundary patches.");
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
