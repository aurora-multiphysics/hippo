#include "FoamPostprocessorBCBase.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "PostprocessorInterface.h"

class FoamMassFlowRateInletBC : public FoamPostprocessorBCBase
{
public:
  static InputParameters validParams();

  FoamMassFlowRateInletBC(const InputParameters & params);

  virtual void imposeBoundaryCondition() override;
};
