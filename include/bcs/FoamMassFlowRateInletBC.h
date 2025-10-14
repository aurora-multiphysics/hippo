#include "FoamBCBase.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "PostprocessorInterface.h"

class FoamMassFlowRateInletBC : public FoamBCBase, public PostprocessorInterface
{
public:
  static InputParameters validParams();

  FoamMassFlowRateInletBC(const InputParameters & params);

  virtual void imposeBoundaryCondition() override;

  virtual void initialSetup() override;

protected:
  PostprocessorName _pp_name;
};
