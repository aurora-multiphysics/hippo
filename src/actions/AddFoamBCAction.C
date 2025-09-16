#include "AddFoamBCAction.h"
#include "InputParameters.h"
#include "MooseObjectAction.h"
#include "FoamProblem.h"

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
  if (_current_task == "add_foam_bc")
  {
    auto foam_problem = dynamic_cast<FoamProblem *>(_problem.get());

    if (!foam_problem)
      mooseError("[FoamBC] can only be used with FoamProblem.");

    std::cout << "FoamBC action placeholder\n";
  }
}
