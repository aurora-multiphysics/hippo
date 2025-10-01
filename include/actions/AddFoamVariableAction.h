#include "FoamProblem.h"
#include "InputParameters.h"
#include "MooseObjectAction.h"

class AddFoamVariableAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddFoamVariableAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  // Function to enable compatibility with old style variable shadowing
  void addOldStyleVariables(FoamProblem & problem);

  // Create AuxVariable associated with new-style variable shadowing
  void createAuxVariable();
};
