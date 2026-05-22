#include "FoamDiffusionFluxPostprocessorBC.h"
#include "FoamPostprocessorBCBase.h"
#include "PstreamReduceOps.H"
#include "Registry.h"
#include <algorithm>

registerMooseObject("hippoApp", FoamDiffusionFluxPostprocessorBC);

InputParameters
FoamDiffusionFluxPostprocessorBC::validParams()
{
  auto params = FoamPostprocessorBCBase::validParams();
  params.addParam<std::string>(
      "diffusivity", "kappa", "Diffusivity for BC, defaults to kappa, the thermal conducitivity.");
  return params;
}

FoamDiffusionFluxPostprocessorBC::FoamDiffusionFluxPostprocessorBC(const InputParameters & params)
  : FoamPostprocessorBCBase(params), _diffusivity(getParam<std::string>("diffusivity"))
{
  if (!_mesh->fvMesh().foundObject<Foam::volScalarField>(_diffusivity))
  {
    mooseError("Diffusivity '", _diffusivity, "' not a Foam volScalarField.");
  }
}

void
FoamDiffusionFluxPostprocessorBC::imposeBoundaryCondition()
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

    // Get the underlying diffusivity field
    auto & coeff = foam_mesh.boundary()[subdomain].lookupPatchField<Foam::volScalarField, double>(
        _diffusivity);

    // Calculate the bulk value of the diffusivity coefficient
    auto area = boundary.magSf();
    auto total_area = Foam::returnReduce(Foam::sum(area), Foam::sumOp<Foam::scalar>());
    auto coeff_bulk =
        Foam::returnReduce(Foam::sum(coeff * area), Foam::sumOp<Foam::scalar>()) / total_area;

    // set gradient
    std::fill(foam_gradient.begin(), foam_gradient.end(), _pp_value / coeff_bulk);
  }
}
