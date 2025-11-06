#include "FoamVariableBCBase.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "PostprocessorInterface.h"

class FoamMassFlowRateInletBC : public FoamVariableBCBase, public PostprocessorInterface
{
public:
  static InputParameters validParams();

  FoamMassFlowRateInletBC(const InputParameters & params);

  virtual void imposeBoundaryCondition() override;

  virtual void initialSetup() override;

protected:
  PostprocessorName _pp_name;
};
