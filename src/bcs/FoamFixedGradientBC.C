#include "FoamFixedGradientBC.h"

#include <InputParameters.h>
#include <MooseTypes.h>
#include <fixedGradientFvPatchFields.H>
#include <volFieldsFwd.H>

registerMooseObject("hippoApp", FoamFixedGradientBC);

InputParameters
FoamFixedGradientBC::validParams()
{
  auto params = FoamBCBase::validParams();
  params.addClassDescription("A FoamBC that imposes a fixed gradient dirichlet boundary condition "
                             "on the OpenFOAM simulation");
  params.addParam<std::string>("diffusivity_coefficient",
                               "OpenFOAM scalar field name to be specified if 'v' is "
                               "a flux rather than a gradient");
  return params;
}

FoamFixedGradientBC::FoamFixedGradientBC(const InputParameters & parameters)
  : FoamBCBase(parameters),
    _diffusivity_coefficient(parameters.get<std::string>("diffusivity_coefficient"))
{
  // check that the diffusivity coefficient is a OpenFOAM scalar field
  if (!_diffusivity_coefficient.empty() &&
      !_mesh->foamHasObject<Foam::volScalarField>(_diffusivity_coefficient))
    mooseError(
        "Diffusivity coefficient '", _diffusivity_coefficient, "' not a Foam volScalarField");
}

void
FoamFixedGradientBC::imposeBoundaryCondition()
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

    // If diffusivity_coefficient is specified grad array is a flux, so result
    // must be divided by it
    if (!_diffusivity_coefficient.empty())
    {
      // Get the underlying diffusivity field
      auto & coeff = foam_mesh.boundary()[subdomain].lookupPatchField<Foam::volScalarField, double>(
          _diffusivity_coefficient);

      assert(foam_gradient.size() == coeff.size());
      // set gradient
      for (auto i = 0; i < foam_gradient.size(); ++i)
      {
        foam_gradient[i] = grad_array[i] / coeff[i];
      }
    }
    else // if no diffusivity coefficient grad_array is just the gradient so copy
    {
      std::copy(grad_array.begin(), grad_array.end(), foam_gradient.begin());
    }
  }
}
