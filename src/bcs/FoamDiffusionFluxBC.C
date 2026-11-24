
#include "FoamBCBase.h"
#include "FoamDiffusionFluxBC.h"
#include "FoamVariableBCBase.h"
#include "MooseError.h"

#include <InputParameters.h>
#include <MooseTypes.h>
#include <fixedGradientFvPatchFields.H>
#include <volFieldsFwd.H>

registerMooseObject("hippoApp", FoamDiffusionFluxBC);

InputParameters
FoamDiffusionFluxBC::validParams()
{
  auto params = FoamVariableBCBase::validParams();
  params.addParam<std::string>(
      "diffusivity", "kappa", "Diffusivity for BC, defaults to kappa, the thermal conducitivity.");

  params.addClassDescription("A FoamBC that imposes a fixed gradient boundary condition "
                             "on the OpenFOAM simulation");
  return params;
}

FoamDiffusionFluxBC::FoamDiffusionFluxBC(const InputParameters & params)
  : FoamVariableBCBase(params, FoamBCType::fixedGradient),
    _diffusivity(getParam<std::string>("diffusivity"))
{
  if (!getFvMesh().foundObject<Foam::volScalarField>(_diffusivity))
  {
    mooseError("Diffusivity '", _diffusivity, "' not a Foam volScalarField.");
  }
}

void
FoamDiffusionFluxBC::imposeBoundaryCondition()
{
  // Get subdomains this FoamBC acts on
  // TODO: replace with BoundaryRestriction member functions once FoamMesh is updated
  auto subdomains = getFoamMesh().getSubdomainIDs(_boundary);
  for (auto subdomain : subdomains)
  {
    std::vector<Real> && grad_array = getMooseVariableArray(subdomain);

    // Get the gradient associated with the field
    auto & foam_gradient =
        getFoamMesh().getGradientBCField<Foam::volScalarField, double>(subdomain, _foam_variable);
    assert(grad_array.size() == static_cast<size_t>(foam_gradient.size()));

    auto & coeff = getFvMesh().boundary()[subdomain].lookupPatchField<Foam::volScalarField, double>(
        _diffusivity);

    assert(foam_gradient.size() == coeff.size());
    // set gradient
    for (auto i = 0; i < foam_gradient.size(); ++i)
    {
      foam_gradient[i] = grad_array[i] / coeff[i];
    }
  }
}
