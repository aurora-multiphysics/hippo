#pragma once

#include "FoamVariableBCBase.h"
#include "InputParameters.h"

class FoamFixedValueBC : public FoamVariableBCBase
{
public:
  // Validate input file parameters
  static InputParameters validParams();

  // Constructor
  explicit FoamFixedValueBC(const InputParameters & parameters);

  // Impose boundary conditions (to be called from FoamProblem class)
  virtual void imposeBoundaryCondition() override;
};
