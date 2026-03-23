#include "FoamFixedGradientPostprocessorBC.h"
#include "PstreamReduceOps.H"
#include "Registry.h"
#include <algorithm>

registerMooseObject("hippoApp", FoamFixedGradientPostprocessorBC);

InputParameters
FoamFixedGradientPostprocessorBC::validParams()
{
  auto params = FoamPostprocessorBCBase::validParams();
  params.addParam<std::string>("diffusivity_coefficient",
                               "",
                               "OpenFOAM scalar field name to be specified if 'v' is "
                               "a flux rather than a gradient");
  return params;
}

FoamFixedGradientPostprocessorBC::FoamFixedGradientPostprocessorBC(const InputParameters & params)
  : FoamPostprocessorBCBase(params),
    _diffusivity_coefficient(params.get<std::string>("diffusivity_coefficient"))
{
  // check that the diffusivity coefficient is a OpenFOAM scalar field
  if (!_diffusivity_coefficient.empty() &&
      !_mesh->fvMesh().foundObject<Foam::volScalarField>(_diffusivity_coefficient))
    mooseError(
        "Diffusivity coefficient '", _diffusivity_coefficient, "' not a Foam volScalarField");
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
    if (!_diffusivity_coefficient.empty())
    {
      // Get the underlying diffusivity field
      auto & coeff = foam_mesh.boundary()[subdomain].lookupPatchField<Foam::volScalarField, double>(
          _diffusivity_coefficient);

      // Calculate the bulk value of the diffusivity coefficient
      auto area = boundary.magSf();
      auto total_area = Foam::returnReduce(Foam::sum(area), Foam::sumOp<Foam::scalar>());
      auto coeff_bulk =
          Foam::returnReduce(Foam::sum(coeff * area), Foam::sumOp<Foam::scalar>()) / total_area;

      // set gradient
      std::fill(foam_gradient.begin(), foam_gradient.end(), _pp_value / coeff_bulk);
    }
    else // if no diffusivity coefficient grad_array is just the gradient so fill
    {
      std::fill(foam_gradient.begin(), foam_gradient.end(), _pp_value);
    }
  }
}
