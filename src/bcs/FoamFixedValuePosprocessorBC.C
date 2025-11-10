#include "FoamFixedValuePostprocessorBC.h"
#include "Registry.h"

registerMooseObject("hippoApp", FoamFixedValuePostprocessorBC);

InputParameters
FoamFixedValuePostprocessorBC::validParams()
{
  return FoamPostprocessorBCBase::validParams();
}

FoamFixedValuePostprocessorBC::FoamFixedValuePostprocessorBC(const InputParameters & params)
  : FoamPostprocessorBCBase(params)
{
}

void
FoamFixedValuePostprocessorBC::imposeBoundaryCondition()
{
  auto & foam_mesh = _mesh->fvMesh();

  // Get subdomains this FoamBC acts on
  // TODO: replace with BoundaryRestriction member functions once FoamMesh is updated
  auto subdomains = _mesh->getSubdomainIDs(_boundary);
  for (auto subdomain : subdomains)
  {
    // Get underlying field from OpenFOAM boundary patch
    auto & foam_var = const_cast<Foam::fvPatchField<double> &>(
        foam_mesh.boundary()[subdomain].lookupPatchField<Foam::volScalarField, double>(
            _foam_variable));

    std::fill(foam_var.begin(), foam_var.end(), _pp_value);
  }
}
