#include "FoamFixedGradientBC.h"

#include <InputParameters.h>
#include <MooseTypes.h>
#include <fixedGradientFvPatchFields.H>
#include <volFieldsFwd.H>

registerMooseObject("hippoApp", FoamFixedGradientBC);

InputParameters
FoamFixedGradientBC::validParams()
{
  auto params = FoamVariableBCBase::validParams();
  params.addClassDescription("A FoamBC that imposes a fixed gradient boundary condition "
                             "on the OpenFOAM simulation");
  return params;
}

FoamFixedGradientBC::FoamFixedGradientBC(const InputParameters & parameters)
  : FoamVariableBCBase(parameters)
{
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

    std::copy(grad_array.begin(), grad_array.end(), foam_gradient.begin());
  }
}
