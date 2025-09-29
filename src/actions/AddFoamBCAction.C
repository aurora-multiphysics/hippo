#include "AddFoamBCAction.h"
#include "InputParameters.h"
#include "MooseObjectAction.h"
#include "FoamProblem.h"
#include "FoamFixedValueBC.h"
#include "FoamFixedGradientBC.h"

registerMooseAction("hippoApp", AddFoamBCAction, "add_foam_bc");
registerMooseAction("hippoApp", AddFoamBCAction, "check_deprecated_bc");

InputParameters
AddFoamBCAction::validParams()
{
  return MooseObjectAction::validParams();
}

AddFoamBCAction::AddFoamBCAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddFoamBCAction::act()
{
  auto foam_problem = dynamic_cast<FoamProblem *>(_problem.get());

  if (_current_task == "add_foam_bc")
  {
    if (!foam_problem)
      mooseError("FoamBCs system can only be used with FoamProblem.");

    if (foam_problem->hasOldBCSyntax())
      mooseError(
          "Old BC syntax (Problem/temp, Problem/heat_flux) cannot be used with FoamBCs system");

    foam_problem->addObject<FoamBCBase>(_type, _name, _moose_object_pars, false);
  }
  else if (_current_task == "check_deprecated_bc")
  {
    if (!foam_problem)
      return;
    addOldStyleBCs(*foam_problem);
  }
}

void
AddFoamBCAction::addOldStyleBCs(FoamProblem & problem)
{

  auto & problem_params = problem.parameters();
  if (problem_params.isParamSetByUser("temp"))
  {
    if (problem_params.isParamSetByUser("heat_flux"))
      mooseError("'temp' and 'heat_flux' cannot both be specified.");

    auto params = _factory.getValidParams("FoamFixedValueBC");
    params.set<std::string>("foam_variable") = 'T';
    params.set<std::vector<VariableName>>("v") = {
        VariableName(problem_params.get<std::string>("temp"))};
    params.set<FEProblemBase *>("_fe_problem_base") = &problem;

    problem.addObject<FoamFixedValueBC>("FoamFixedValueBC", "temp", params);
  }
  else if (problem_params.isParamSetByUser("heat_flux"))
  {
    auto params = _factory.getValidParams("FoamFixedGradientBC");
    params.set<std::string>("foam_variable") = 'T';
    params.set<std::vector<VariableName>>("v") = {
        VariableName(problem_params.get<std::string>("heat_flux"))};
    params.set<std::string>("diffusivity_coefficient") = "kappa";

    params.set<FEProblemBase *>("_fe_problem_base") = &problem;
    problem.addObject<FoamFixedGradientBC>("FoamFixedGradientBC", "heat_flux", params);
  }
}
