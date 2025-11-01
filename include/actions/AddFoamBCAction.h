#pragma once

#include "InputParameters.h"
#include "MooseObjectAction.h"

class AddFoamBCAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddFoamBCAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  // Create AuxVariable associated with new-style BCs
  void createAuxVariable();
};
