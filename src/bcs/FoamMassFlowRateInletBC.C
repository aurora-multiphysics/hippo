#include "FoamBCBase.h"
#include "FoamMassFlowRateInletBC.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "Postprocessor.h"
#include "Registry.h"
#include "finiteVolume/fields/fvPatchFields/derived/flowRateInletVelocity/flowRateInletVelocityFvPatchVectorField.H"

registerMooseObject("hippoApp", FoamMassFlowRateInletBC);

InputParameters
FoamMassFlowRateInletBC::validParams()
{
  auto params = FoamBCBase::validParams();
  params.remove("v");
  params.remove("initial_condition");
  params.remove("foam_variable");

  params.addParam<std::string>(
      "foam_variable", "U", "Name of foam variable associated with velocity");

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

    auto & U_var = const_cast<Foam::fvPatchField<Foam::vector> &>(
        foam_mesh.boundary()[subdomain].lookupPatchField<Foam::volVectorField, double>(
            _foam_variable));
    auto && nf = foam_mesh.boundary()[subdomain].nf();
    auto & rho =
        foam_mesh.boundary()[subdomain].lookupPatchField<Foam::volScalarField, double>("rho");

    U_var = nf / rho * pp_value;
  }
}

void
FoamMassFlowRateInletBC::initialSetup()
{
}
