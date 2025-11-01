#include "AddFoamBCAction.h"
#include "FoamProblem.h"
#include "hippoUtils.h"

#include <InputParameters.h>
#include <MooseObjectAction.h>

registerMooseAction("hippoApp", AddFoamBCAction, "add_foam_bc");

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

    // Do not create aux variable if variable provided.
    if (!_moose_object_pars.isParamSetByUser("v"))
      createAuxVariable();

    foam_problem->addObject<FoamBCBase>(_type, _name, _moose_object_pars, false);
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
  Hippo::internal::copyParamFromParam<std::vector<Real>>(
      action_params, _moose_object_pars, "initial_condition");

  std::shared_ptr<Action> action =
      std::static_pointer_cast<Action>(_action_factory.create(class_name, name(), action_params));
  _awh.addActionBlock(action);
}
