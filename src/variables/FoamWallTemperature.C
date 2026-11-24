#include "FoamWallTemperature.h"
#include "InputParameters.h"

registerMooseObject("hippoApp", FoamWallTemperature);

InputParameters
FoamWallTemperature::validParams()
{
  InputParameters params = FoamWallVariableBase::validParams();
  params.addClassDescription(
      "Transfers wall temperature from the selected OpenFOAM boundaries to a MOOSE "
      "variable. For multiphase flow, temperature is volume-fraction weighted.");

  return params;
}

FoamWallTemperature::FoamWallTemperature(const InputParameters & params)
  : FoamWallVariableBase(params)
{
}

const Foam::scalarField
FoamWallTemperature::getFoamField(const SubdomainName & boundary)
{
  return _wall_quantities->wallTemperature(boundary);
}
