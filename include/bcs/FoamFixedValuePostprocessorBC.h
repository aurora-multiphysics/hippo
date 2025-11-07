#pragma once

#include "FoamPostprocessorBCBase.h"
#include "InputParameters.h"

class FoamFixedValuePostprocessorBC : public FoamPostprocessorBCBase
{
public:
  static InputParameters validParams();

  FoamFixedValuePostprocessorBC(const InputParameters & params);

  virtual void imposeBoundaryCondition() override;
};
