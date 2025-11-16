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

  params.addParam<Real>("scale_factor", 1., "Scale factor multiply mass flow rate pp by.");
  params.remove("foam_variable");
  params.addPrivateParam<std::string>("foam_variable", "U");

  return params;
}

FoamMassFlowRateInletBC::FoamMassFlowRateInletBC(const InputParameters & params)
  : FoamPostprocessorBCBase(params), _scale_factor(params.get<Real>("scale_factor"))
{
}

void
FoamMassFlowRateInletBC::imposeBoundaryCondition()
{
  auto & foam_mesh = _mesh->fvMesh();

  // Get subdomains this FoamBC acts on
  // TODO: replace with BoundaryRestriction member functions once FoamMesh is updated
  auto subdomains = _mesh->getSubdomainIDs(_boundary);
  for (auto subdomain : subdomains)
  {
    auto & boundary_patch = foam_mesh.boundary()[subdomain];

    auto & U_var = const_cast<Foam::fvPatchField<Foam::vector> &>(
        boundary_patch.lookupPatchField<Foam::volVectorField, double>("U"));
    auto & rho = boundary_patch.lookupPatchField<Foam::volScalarField, double>("rho");
    Real area = Foam::returnReduce(Foam::sum(boundary_patch.magSf()), Foam::sumOp<Real>());
    U_var == -_scale_factor * _pp_value * boundary_patch.nf() / (rho * area);
  }
}
