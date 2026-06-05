
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
  : FoamVariableBCBase(params), _diffusivity(getParam<std::string>("diffusivity"))
{
  if (!_mesh->fvMesh().foundObject<Foam::volScalarField>(_diffusivity))
  {
    mooseError("Diffusivity '", _diffusivity, "' not a Foam volScalarField.");
  }
}

void
FoamDiffusionFluxBC::imposeBoundaryCondition()
{
  auto & foam_mesh = _mesh->fvMesh();

  // Get subdomains this FoamBC acts on
  // TODO: replace with BoundaryRestriction member functions once FoamMesh is updated
  auto subdomains = _mesh->getSubdomainIDs(_boundary);
  for (auto subdomain : subdomains)
  {
    std::vector<Real> && grad_array = getMooseVariableArray(subdomain);

    // Get the gradient associated with the field
    auto & foam_gradient =
        _mesh->getGradientBCField<Foam::volScalarField, double>(subdomain, _foam_variable);
    assert(grad_array.size() == static_cast<size_t>(foam_gradient.size()));

    auto & coeff = foam_mesh.boundary()[subdomain].lookupPatchField<Foam::volScalarField, double>(
        _diffusivity);

    assert(foam_gradient.size() == coeff.size());
    // set gradient
    for (auto i = 0; i < foam_gradient.size(); ++i)
    {
      foam_gradient[i] = grad_array[i] / coeff[i];
    }
  }
}
