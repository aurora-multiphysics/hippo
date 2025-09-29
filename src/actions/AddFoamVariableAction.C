#include "AddFoamVariableAction.h"
#include "FoamProblem.h"
#include "FoamVariableField.h"
#include "FoamFunctionObject.h"
#include "InputParameters.h"
#include "MooseObjectAction.h"
#include "Registry.h"

registerMooseAction("hippoApp", AddFoamVariableAction, "add_foam_variable");
registerMooseAction("hippoApp", AddFoamVariableAction, "check_deprecated_var");

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
  auto * foam_problem = dynamic_cast<FoamProblem *>(_problem.get());

  // Add variable through [FoamVariables] block
  if (_current_task == "add_foam_variable")
  {
    if (!foam_problem)
      mooseError("FoamVariables system can only be used with FoamProblem.");

    if (foam_problem->hasOldVariableSyntax())
      mooseError("The old variable shadowing sytax (Problem/foam_temp, Problem/foam_heat_flux) "
                 "cannot be used with FoamVariables system");

    foam_problem->addObject<FoamVariableField>(_type, _name, _moose_object_pars, false);
  }
  else if (_current_task == "check_deprecated_var")
  {
    if (!foam_problem)
      return;
    addOldStyleVariables(*foam_problem);
  }
}

void
AddFoamVariableAction::addOldStyleVariables(FoamProblem & problem)
{
  auto & problem_params = problem.parameters();

  // construct new-style Variable from old-style foam_temp and foam_heat_flux parameters
  if (problem_params.isParamSetByUser("foam_temp"))
  {
    auto params = _factory.getValidParams("FoamVariableField");
    params.set<std::string>("foam_variable") = 'T';

    // In the old style, the MooseVariable has already been created so this tell the
    // FoamVariableField to get the existing field rather than create a new one.
    params.set<bool>("_deprecated") = true;

    problem.addObject<FoamVariableField>(
        "FoamVariableField", problem_params.get<std::string>("foam_temp"), params);
  }

  if (problem_params.isParamSetByUser("foam_heat_flux"))
  {
    auto params = _factory.getValidParams("FoamFunctionObject");
    params.set<std::string>("foam_variable") = "wallheatFlux";
    params.set<bool>("_deprecated") = true;

    problem.addObject<FoamFunctionObject>(
        "FoamFunctionObject", problem_params.get<std::string>("foam_heat_flux"), params);
  }
}
