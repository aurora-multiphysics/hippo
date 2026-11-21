#include "FoamDiffusionFluxBC.h"

registerMooseObject("hippoApp", FoamDiffusionFluxBC);

InputParameters
FoamDiffusionFluxBC::validParams()
{
  auto params = FoamVariableBCBase::validParams();
  params.addParam<std::string>(
      "diffusivity", "kappa", "Diffusivity for BC, defaults to kappa, the thermal conductivity.");

  params.addClassDescription("A FoamBC that imposes a prescribed diffusion flux "
                             "on the OpenFOAM simulation");
  return params;
}

FoamDiffusionFluxBC::FoamDiffusionFluxBC(const InputParameters & params)
  : FoamVariableBCBase(params, FoamBCType::diffusionFlux)
{
  const auto & diffusivity = getParam<std::string>("diffusivity");
  if (!getFvMesh().foundObject<Foam::volScalarField>(diffusivity))
    paramError("diffusivity", "Field '", diffusivity, "' must be an OpenFOAM volScalarField.");
}
