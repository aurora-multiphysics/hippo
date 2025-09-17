#include "FoamFixedValueBC.h"
#include "InputParameters.h"
#include "MooseTypes.h"

registerMooseObject("hippoApp", FoamFixedValueBC);

InputParameters
FoamFixedValueBC::validParams()
{
  return FoamBCBase::validParams();
}

FoamFixedValueBC::FoamFixedValueBC(const InputParameters & parameters) : FoamBCBase(parameters) {}

void
FoamFixedValueBC::imposeBoundaryCondition()
{
  auto subdomains = _mesh->getSubdomainList();
  auto & foam_mesh = _mesh->fvMesh();
  for (auto subdomain : subdomains)
  {
    std::vector<Real> && var_array = getVariableArray(subdomain);

    auto & foam_var = const_cast<Foam::fvPatchField<double> &>(
        foam_mesh.boundary()[subdomain].lookupPatchField<Foam::volScalarField, double>("T"));

    assert(moose_t.size() == static_cast<std::size_t>(temp.size()));

    std::copy(var_array.begin(), var_array.end(), foam_var.begin());
  }
}
