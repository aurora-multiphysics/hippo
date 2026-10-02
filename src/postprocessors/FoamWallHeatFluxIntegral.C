#include "FoamWallHeatFluxIntegral.h"

registerMooseObject("hippoApp", FoamWallHeatFluxIntegral);

InputParameters
FoamWallHeatFluxIntegral::validParams()
{
  InputParameters params = FoamWallPostprocessor::validParams();
  params.addClassDescription(
      "Computes the surface integral of wall heat flux over the selected OpenFOAM boundary "
      "patches, giving the heat transfer rate in W, positive into the fluid.");
  return params;
}

FoamWallHeatFluxIntegral::FoamWallHeatFluxIntegral(const InputParameters & params)
  : FoamWallPostprocessor(params)
{
}

void
FoamWallHeatFluxIntegral::compute()
{
  _value = integrateField();
}
