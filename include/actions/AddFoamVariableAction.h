#include "InputParameters.h"
#include "MooseObjectAction.h"

class AddFoamVariableAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddFoamVariableAction(const InputParameters & parameters);

  virtual void act() override;
};
