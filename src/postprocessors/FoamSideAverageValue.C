#include "FoamSideAverageValue.h"
#include "InputParameters.h"

registerMooseObject("hippoApp", FoamSideAverageValue);

FoamSideAverageValue::FoamSideAverageValue(const InputParameters & params)
  : FoamSideIntegratedValue(params)
{
}

void
FoamSideAverageValue::compute()
{
  _value = integrateValue() / getArea();
}
