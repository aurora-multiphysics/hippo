#include "DimensionedField.H"
#include "FoamBCBase.h"
#include "FoamMassFlowRateInletBC.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "PstreamReduceOps.H"
#include "Registry.h"
#include <numeric>

registerMooseObject("hippoApp", FoamMassFlowRateInletBC);

InputParameters
FoamMassFlowRateInletBC::validParams()
{
  auto params = FoamBCBase::validParams();
  params.remove("v");
  params.remove("initial_condition");
  params.remove("foam_variable");

  params.addParam<std::string>(
      "foam_variable", "T", "Name of foam variable associated with velocity");

  params.addRequiredParam<PostprocessorName>("postprocessor",
                                             "Postprocessors containing mass flow rate.");

  return params;
}

FoamMassFlowRateInletBC::FoamMassFlowRateInletBC(const InputParameters & params)
  : FoamBCBase(params),
    PostprocessorInterface(this),
    _pp_name(params.get<PostprocessorName>("postprocessor"))
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
    auto pp_value = getPostprocessorValueByName(_pp_name);
    auto & boundary_patch = foam_mesh.boundary()[subdomain];

    auto & U_var = const_cast<Foam::fvPatchField<Foam::vector> &>(
        boundary_patch.lookupPatchField<Foam::volVectorField, double>("U"));
    auto & rho = boundary_patch.lookupPatchField<Foam::volScalarField, double>("rho");
    Real area = Foam::returnReduce(Foam::sum(boundary_patch.magSf()), Foam::sumOp<Real>());
    U_var == -pp_value * boundary_patch.nf() / (rho * area);
  }
}

void
FoamMassFlowRateInletBC::initialSetup()
{
  _foam_variable = "";
}
