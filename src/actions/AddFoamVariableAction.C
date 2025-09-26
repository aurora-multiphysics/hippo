#include "AddFoamVariableAction.h"
#include "FoamProblem.h"
#include "FoamVariableField.h"
#include "InputParameters.h"
#include "MooseObjectAction.h"
#include "Registry.h"

registerMooseAction("hippoApp", AddFoamVariableAction, "add_foam_variable");

InputParameters
AddFoamVariableAction::validParams()
{
  auto params = MooseObjectAction::validParams();
  params.addClassDescription("Adds a FoamVariable that shadows an OpenFOAM scalar field.");
  return params;
}

AddFoamVariableAction::AddFoamVariableAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddFoamVariableAction::act()
{
  if (_current_task == "add_foam_variable")
  {
    auto * foam_problem = dynamic_cast<FoamProblem *>(_problem.get());
    if (!foam_problem)
      mooseError("[FoamVariables] can only be used with FoamProblem.");

    if (foam_problem->oldVariableSyntax())
      mooseError("The old variable shadowing sytax (Problem/foam_temp, Problem/foam_heat_flux) "
                 "cannot be used with [FoamVariables] block");

    foam_problem->addObject<FoamVariableField>(_type, _name, _moose_object_pars, false);
  }
}
