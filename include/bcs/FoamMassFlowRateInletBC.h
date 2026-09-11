#include "FoamPostprocessorBCBase.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "PostprocessorInterface.h"

class FoamMassFlowRateInletBC : public FoamPostprocessorBCBase
{
public:
  static InputParameters validParams();

  FoamMassFlowRateInletBC(const InputParameters & params);

  virtual void imposeBoundaryCondition(bool initialisation = false) override;

protected:
  void constructFoamPatch(Foam::label patch_id) override { constructFixedValuePatch(patch_id); }

  const Real _scale_factor;
};
