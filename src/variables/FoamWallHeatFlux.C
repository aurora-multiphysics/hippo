#include "FoamWallHeatFlux.h"
#include "InputParameters.h"
#include "Registry.h"
#include <memory>

registerMooseObject("hippoApp", FoamWallHeatFlux);

InputParameters
FoamWallHeatFlux::validParams()
{
  InputParameters params = FoamWallVariableBase::validParams();
  params.addClassDescription(
      "Transfers wall heat flux from the selected OpenFOAM boundaries to a MOOSE "
      "variable. For multiphase flow, the flux includes contributions from all phases. "
      "Positive values indicate heat entering the fluid.");

  return params;
}

FoamWallHeatFlux::FoamWallHeatFlux(const InputParameters & params) : FoamWallVariableBase(params) {}

const Foam::scalarField
FoamWallHeatFlux::getFoamField(const SubdomainName & boundary)
{
  return _wall_quantities->wallHeatFlux(boundary);
}
