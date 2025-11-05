#pragma once
#include "FoamMappedInletBCBase.h"

class FoamMassFlowRateMappedInletBC : public FoamMappedInletBCBase
{
public:
  static InputParameters validParams();

  FoamMassFlowRateMappedInletBC(const InputParameters & params);

  virtual void imposeBoundaryCondition() override;
};
