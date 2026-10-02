#include "FoamMassFlowRateInletBC.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "PstreamReduceOps.H"
#include "Registry.h"

registerMooseObject("hippoApp", FoamMassFlowRateInletBC);

InputParameters
FoamMassFlowRateInletBC::validParams()
{
  auto params = FoamPostprocessorBCBase::validParams();

  params.addParam<Real>("scale_factor", 1., "Scale factor multiply mass flow rate pp_name by.");
  params.suppressParameter<std::string>("foam_variable");
  params.set<std::string>("foam_variable") = "U";

  return params;
}

FoamMassFlowRateInletBC::FoamMassFlowRateInletBC(const InputParameters & params)
  : FoamPostprocessorBCBase(params), _scale_factor(params.get<Real>("scale_factor"))
{
}

void
FoamMassFlowRateInletBC::imposeBoundaryCondition(bool initialisation)
{
  // Get subdomains this FoamBC acts on
  // TODO: replace with BoundaryRestriction member functions once FoamMesh is updated
  auto subdomains = getFoamMesh().getSubdomainIDs(_boundary);
  for (auto subdomain : subdomains)
  {
    const auto & boundary_patch = getFvMesh().boundary()[subdomain];

    auto & U_var = const_cast<Foam::fvPatchField<Foam::vector> &>(
        boundary_patch.lookupPatchField<Foam::volVectorField, double>("U"));
    const auto & rho = boundary_patch.lookupPatchField<Foam::volScalarField, double>("rho");
    const Real area = Foam::returnReduce(Foam::sum(boundary_patch.magSf()), Foam::sumOp<Real>());

    auto value = -_scale_factor * _pp_value * boundary_patch.nf() / (rho * area);
    updateBC(U_var, value(), initialisation);
  }
}
