#pragma once
#include "FoamMappedInletBCBase.h"
#include "MooseEnum.h"

class FoamMassFlowRateMappedInletBC : public FoamMappedInletBCBase
{
public:
  static InputParameters validParams();

  FoamMassFlowRateMappedInletBC(const InputParameters & params);

  virtual void imposeBoundaryCondition() override;

protected:
  MooseEnum _scale_method;

  Real _scale_factor;
};
