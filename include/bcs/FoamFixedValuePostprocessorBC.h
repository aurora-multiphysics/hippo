#pragma once

#include "FoamPostprocessorBCBase.h"

class FoamFixedValuePostprocessorBC : public FoamPostprocessorBCBase
{
public:
  static InputParameters validParams();

  FoamFixedValuePostprocessorBC(const InputParameters & params);

  // Impose boundary conditions (to be called from FoamProblem class)
  virtual void imposeBoundaryCondition() override;
};
