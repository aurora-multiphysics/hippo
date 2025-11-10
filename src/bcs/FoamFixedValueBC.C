#include "FoamFixedValueBC.h"
#include "InputParameters.h"
#include "MooseTypes.h"

registerMooseObject("hippoApp", FoamFixedValueBC);

InputParameters
FoamFixedValueBC::validParams()
{
  auto params = FoamBCBase::validParams();
  params.addClassDescription("A FoamBC that imposes a fixed value dirichlet boundary condition "
                             "on the OpenFOAM simulation");
  return params;
}

FoamFixedValueBC::FoamFixedValueBC(const InputParameters & parameters) : FoamBCBase(parameters) {}

void
FoamFixedValueBC::imposeBoundaryCondition()
{
  auto & foam_mesh = _mesh->fvMesh();

  // Get subdomains this FoamBC acts on
  // TODO: replace with BoundaryRestriction member functions once FoamMesh is updated
  auto subdomains = _mesh->getSubdomainIDs(_boundary);
  for (auto subdomain : subdomains)
  {
    std::vector<Real> && var_array = getMooseVariableArray(subdomain);

    // Get underlying field from OpenFOAM boundary patch
    auto & foam_var = const_cast<Foam::fvPatchField<double> &>(
        foam_mesh.boundary()[subdomain].lookupPatchField<Foam::volScalarField, double>(
            _foam_variable));

    assert(var_array.size() == static_cast<std::size_t>(foam_var.size()));

    std::copy(var_array.begin(), var_array.end(), foam_var.begin());
  }
}
