#include "FoamFixedValuePostprocessorBC.h"
#include "Registry.h"

registerMooseObject("hippoApp", FoamFixedValuePostprocessorBC);

InputParameters
FoamFixedValuePostprocessorBC::validParams()
{
  auto params = FoamPostprocessorBCBase::validParams();

  return params;
}

FoamFixedValuePostprocessorBC::FoamFixedValuePostprocessorBC(const InputParameters & params)
  : FoamPostprocessorBCBase(params)
{
}

void
FoamFixedValuePostprocessorBC::imposeBoundaryCondition()
{
  // Get subdomains this FoamBC acts on
  auto subdomains = getFoamMesh().getSubdomainIDs(_boundary);
  for (auto subdomain : subdomains)
  {
    // Get underlying field from OpenFOAM boundary patch
    auto & foam_var =
        getFoamMesh().getBCField<Foam::volScalarField, double>(subdomain, _foam_variable);

    updateBC(foam_var, _pp_value);
  }
}
