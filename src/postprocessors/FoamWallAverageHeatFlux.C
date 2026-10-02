#include "FoamWallAverageHeatFlux.h"

registerMooseObject("hippoApp", FoamWallAverageHeatFlux);

InputParameters
FoamWallAverageHeatFlux::validParams()
{
  InputParameters params = FoamWallPostprocessor::validParams();
  params.addClassDescription(
      "Computes the area-weighted average wall heat flux in W/m^2 over the selected OpenFOAM "
      "boundary patches, positive into the fluid.");
  return params;
}

FoamWallAverageHeatFlux::FoamWallAverageHeatFlux(const InputParameters & params)
  : FoamWallPostprocessor(params)
{
}

void
FoamWallAverageHeatFlux::compute()
{
  _value = averageField();
}
