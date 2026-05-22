#include "FoamFixedGradientPostprocessorBC.h"
#include "PstreamReduceOps.H"
#include "Registry.h"
#include <algorithm>

registerMooseObject("hippoApp", FoamFixedGradientPostprocessorBC);

InputParameters
FoamFixedGradientPostprocessorBC::validParams()
{
  auto params = FoamPostprocessorBCBase::validParams();
  return params;
}

FoamFixedGradientPostprocessorBC::FoamFixedGradientPostprocessorBC(const InputParameters & params)
  : FoamPostprocessorBCBase(params)
{
}

void
FoamFixedGradientPostprocessorBC::imposeBoundaryCondition()
{
  auto & foam_mesh = _mesh->fvMesh();

  // Get subdomains this FoamBC acts on
  auto subdomains = _mesh->getSubdomainIDs(_boundary);
  for (auto subdomain : subdomains)
  {
    auto & boundary = foam_mesh.boundary()[subdomain];
    // Get underlying field from OpenFOAM boundary patch.
    auto & foam_gradient =
        _mesh->getGradientBCField<Foam::volScalarField, double>(subdomain, _foam_variable);

    // If diffusivity_coefficient is specified grad array is a flux, so result
    // must be divided by it
    std::fill(foam_gradient.begin(), foam_gradient.end(), _pp_value);
  }
}
