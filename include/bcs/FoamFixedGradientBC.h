#pragma once

#include "FoamVariableBCBase.h"
#include "InputParameters.h"

/// Prescribe scalar outward-normal gradients through direct transfers or a MOOSE AuxVariable.
class FoamFixedGradientBC : public FoamVariableBCBase
{
public:
  // Validate input file parameters
  static InputParameters validParams();

  // Constructor
  explicit FoamFixedGradientBC(const InputParameters & parameters);
};
