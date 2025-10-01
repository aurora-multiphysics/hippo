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

  // Adding BCs using [FoamBC] syntax
  if (_current_task == "add_foam_bc")
  {
    if (!foam_problem)
      mooseError("FoamBCs system can only be used with FoamProblem.");

    if (foam_problem->hasOldBCSyntax())
      mooseError(
          "Old BC syntax (Problem/temp, Problem/heat_flux) cannot be used with FoamBCs system");

    // Do not create aux variable if variable provided.
    if (!_moose_object_pars.isParamSetByUser("v"))
      createAuxVariable();

    foam_problem->addObject<FoamBCBase>(_type, _name, _moose_object_pars, false);
  }
  // Adding BCs using old syntax
  else if (_current_task == "check_deprecated_bc")
  {
    // Invoked on Problem but problem may not be a FoamProblem
    if (!foam_problem)
      return;
    addOldStyleBCs(*foam_problem);
  }
}

void
AddFoamBCAction::createAuxVariable()
{
  // Use AddAuxVariableAction to create AuxVariable that also allows additional
  // parameters such as initial conditions to be set
  const std::string class_name = "AddAuxVariableAction";

  InputParameters action_params = _action_factory.getValidParams(class_name);
  action_params.set<std::string>("task") = "add_aux_variable";

  // The MOOSE variable has to be constant monomial
  action_params.set<MooseEnum>("order") = "CONSTANT";
  action_params.set<MooseEnum>("family") = "MONOMIAL";

  // Copy desired parameters from FoamBC action
  copyParamFromParam<std::vector<Real>>(action_params, _moose_object_pars, "initial_condition");

  std::shared_ptr<Action> action =
      std::static_pointer_cast<Action>(_action_factory.create(class_name, name(), action_params));
  _awh.addActionBlock(action);
}

void
AddFoamBCAction::addOldStyleBCs(FoamProblem & problem)
{
  // Old style uses temp or heat_flux to apply boundary conditions
  auto & problem_params = problem.parameters();
  if (problem_params.isParamSetByUser("temp")) // check is temp is set
  {
    if (problem_params.isParamSetByUser("heat_flux"))
      mooseError("'temp' and 'heat_flux' cannot both be specified.");

    auto var_name = problem_params.get<std::string>("temp");

    // Create the equivalent new BC from info from the old style BC
    auto params = _factory.getValidParams("FoamFixedValueBC");
    params.set<std::string>("foam_variable") = 'T';
    params.set<FEProblemBase *>("_fe_problem_base") = &problem;

    problem.addObject<FoamFixedValueBC>("FoamFixedValueBC", var_name, params);
  }
  else if (problem_params.isParamSetByUser("heat_flux"))
  {
    auto var_name = problem_params.get<std::string>("heat_flux");

    auto params = _factory.getValidParams("FoamFixedGradientBC");
    params.set<std::string>("foam_variable") = 'T';
    params.set<std::string>("diffusivity_coefficient") = "kappa";

    params.set<FEProblemBase *>("_fe_problem_base") = &problem;
    problem.addObject<FoamFixedGradientBC>("FoamFixedGradientBC", var_name, params);
  }
}
