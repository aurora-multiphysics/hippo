#include "FoamWallAverageTemperature.h"
#include "FoamWallPostprocessor.h"
#include "InputParameters.h"

registerMooseObject("hippoApp", FoamWallAverageTemperature);

InputParameters
FoamWallAverageTemperature::validParams()
{
  InputParameters params = FoamWallPostprocessor::validParams();
  params.addClassDescription(
      "Computes the area-weighted average wall temperature in K over the selected OpenFOAM "
      "boundary patches.");
  return params;
}

FoamWallAverageTemperature::FoamWallAverageTemperature(const InputParameters & params)
  : FoamWallPostprocessor(params)
{
}

void
FoamWallAverageTemperature::compute()
{
  _value = averageField();
}
