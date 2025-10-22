#pragma once

#include "FoamBCBase.h"
#include "InputParameters.h"

class FoamFixedValueBC : public FoamBCBase
{
public:
  // Validate input file parameters
  static InputParameters validParams();

  // Constructor
  explicit FoamFixedValueBC(const InputParameters & parameters);

  // Impose boundary conditions (to be called from FoamProblem class)
  virtual void imposeBoundaryCondition() override;
};
