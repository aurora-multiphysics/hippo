#include "AddFoamVariableAction.h"
#include "FEProblemBase.h"
#include "FoamFieldBase.h"
#include "FoamMesh.h"
#include "hippoUtils.h"
#include "processorFvPatch.H"
#include <algorithm>

#include "InputParameters.h"
#include "MooseObjectAction.h"
#include "Registry.h"

registerMooseAction("hippoApp", AddFoamVariableAction, "add_foam_variable");

InputParameters
AddFoamVariableAction::validParams()
{
  auto params = MooseObjectAction::validParams();
  params.addClassDescription("Adds a mirrored OpenFOAM variable or a standalone OpenFOAM field.");
  return params;
}

AddFoamVariableAction::AddFoamVariableAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddFoamVariableAction::act()
{

  // Add variable through [FoamVariables] block
  if (_current_task == "add_foam_variable")
  {
    if (FoamFieldBase::mirrorEnabled(_moose_object_pars))
      createAuxVariable();

    _problem->addObject<FoamFieldBase>(_type, _name, _moose_object_pars, false);
  }
}

void
AddFoamVariableAction::createAuxVariable()
{
  // Use AddAuxVariableAction to create AuxVariable that also allows additional
  // parameters such as initial conditions to be set
  const std::string class_name = "AddAuxVariableAction";

  InputParameters action_params = _action_factory.getValidParams(class_name);
  action_params.set<std::string>("task") = "add_aux_variable";

  // The MOOSE variable has to be constant monomial
  action_params.set<MooseEnum>("order") = "CONSTANT";
  action_params.set<MooseEnum>("family") = "MONOMIAL";

  std::vector<SubdomainName> blocks;
  auto * mesh = dynamic_cast<FoamMesh *>(&_problem->mesh());
  if (!mesh)
    mooseError(_type, " can only be used with FoamMesh.");

  // Whether VolField is being wrapped
  const bool borrowed = _moose_object_pars.isParamValid("foam_variable");
  // boundaries stored by FoamVariable
  const auto & boundaries = _moose_object_pars.get<std::vector<BoundaryName>>("boundary");

  // Mirror physical boundaries present in FoamMesh.
  // Wrapped VolFields expose all of them, while owned storage exposes only selected boundaries.
  for (const auto id : mesh->getSubdomainList())
  {
    const auto & boundary = mesh->getSubdomainName(id);
    if (!Foam::isA<Foam::processorFvPatch>(mesh->fvMesh().boundary()[id]) &&
        (borrowed || std::find(boundaries.begin(), boundaries.end(), boundary) != boundaries.end()))
      blocks.push_back(boundary);
  }

  if (blocks.empty())
    mooseError("Boundary mirroring requires at least one exposed boundary in FoamMesh.");

  // Copy desired parameters from FoamVariable action
  Hippo::internal::copyParamFromParam<std::vector<Real>>(
      action_params, _moose_object_pars, "initial_condition");

  auto action = std::static_pointer_cast<MooseObjectAction>(
      _action_factory.create(class_name, name(), action_params));

  // Set block restrictions on the variable parameters
  action->getObjectParams().set<std::vector<SubdomainName>>("block") = blocks;
  _awh.addActionBlock(action);
}
