#pragma once

#include "InputParameters.h"
#include "MooseObjectAction.h"
#include "FoamProblem.h"

class AddFoamBCAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddFoamBCAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  void addOldStyleBCs(FoamProblem & problem);
};
