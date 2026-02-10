#include "FoamFixedValuePostprocessorBC.h"
#include "Registry.h"

registerMooseObject("hippoApp", FoamFixedValuePostprocessorBC);

InputParameters
FoamFixedValuePostprocessorBC::validParams()
{
  return FoamPostprocessorBCBase::validParams();
}

FoamFixedValuePostprocessorBC::FoamFixedValuePostprocessorBC(const InputParameters & params)
  : FoamPostprocessorBCBase(params)
{
}

void
FoamFixedValuePostprocessorBC::imposeBoundaryCondition()
{
  // Get subdomains this FoamBC acts on
  auto subdomains = _mesh->getSubdomainIDs(_boundary);
  for (auto subdomain : subdomains)
  {
    // Get underlying field from OpenFOAM boundary patch
    auto & foam_var = _mesh->getBCField<Foam::volScalarField, double>(subdomain, _foam_variable);

    std::fill(foam_var.begin(), foam_var.end(), _pp_value);
  }
}
