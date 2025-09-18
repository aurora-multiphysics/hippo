#include "FoamFixedGradientBC.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "fixedGradientFvPatchFields.H"
#include "volFieldsFwd.H"

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
  if (!_mesh->fvMesh().foundObject<Foam::volScalarField>(_diffusivity_coefficient))
    mooseError(
        "Diffusivity coefficient '", _diffusivity_coefficient, "' not a Foam volScalarField.");
}

void
FoamFixedGradientBC::imposeBoundaryCondition()
{
  auto & foam_mesh = _mesh->fvMesh();

  // Get subdomains this FoamBC acts on
  // TODO: replace with BoundaryRestriction member functions once FoamMesh is updated
  auto subdomains =
      (_boundary.size() == 0) ? _mesh->getSubdomainList() : _mesh->getSubdomainIDs(_boundary);
  for (auto subdomain : subdomains)
  {
    std::vector<Real> && grad_array = getMooseVariableArray(subdomain);

    // Get underlying field from OpenFOAM boundary patch
    auto & var = const_cast<Foam::fvPatchField<double> &>(
        foam_mesh.boundary()[subdomain].lookupPatchField<Foam::volScalarField, double>(
            _foam_variable));

    // Get the gradient associated with the field
    Foam::scalarField & var_gradient(
        Foam::refCast<Foam::fixedGradientFvPatchScalarField>(var).gradient());

    // If diffusivity_coefficient is specified grad array is a flux, so result
    // must be divided by it
    if (!_diffusivity_coefficient.empty())
    {
      // Get the underlying diffusivity field
      auto & coeff = foam_mesh.boundary()[subdomain].lookupPatchField<Foam::volScalarField, double>(
          _diffusivity_coefficient);

      assert(temp_gradient.size() == coeff.size());
      // set gradient
      for (auto i = 0; i < var_gradient.size(); ++i)
      {
        var_gradient[i] = grad_array[i] / coeff[i];
      }
    }
    else // if no diffusivity coefficient grad_array is just the gradient so copy
    {
      std::copy(grad_array.begin(), grad_array.end(), var_gradient.begin());
    }
  }
}
