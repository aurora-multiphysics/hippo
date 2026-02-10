#include "FoamFixedValueBC.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include <volFieldsFwd.H>

registerMooseObject("hippoApp", FoamFixedValueBC);

InputParameters
FoamFixedValueBC::validParams()
{
  auto params = FoamVariableBCBase::validParams();
  params.addClassDescription("A FoamBC that imposes a fixed value dirichlet boundary condition "
                             "on the OpenFOAM simulation");
  return params;
}

FoamFixedValueBC::FoamFixedValueBC(const InputParameters & parameters)
  : FoamVariableBCBase(parameters)
{
}

void
FoamFixedValueBC::imposeBoundaryCondition()
{
  // Get subdomains this FoamBC acts on
  auto subdomains = _mesh->getSubdomainIDs(_boundary);
  for (auto subdomain : subdomains)
  {
    std::vector<Real> && var_array = getMooseVariableArray(subdomain);

    // Get underlying field from OpenFOAM boundary patch
    auto & foam_var = _mesh->getBCField<Foam::volScalarField, double>(subdomain, _foam_variable);

    assert(var_array.size() == static_cast<std::size_t>(foam_var.size()));

    std::copy(var_array.begin(), var_array.end(), foam_var.begin());
  }
}
