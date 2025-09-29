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
  if (problem_params.isParamSetByUser("foam_temp"))
  {
    auto params = _factory.getValidParams("FoamVariableField");
    params.set<std::string>("foam_variable") = 'T';
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
