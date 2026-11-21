#pragma once

#include "FoamVariableBCBase.h"
#include "InputParameters.h"

/// Prescribe scalar patch values through direct transfers or a MOOSE AuxVariable.
class FoamFixedValueBC : public FoamVariableBCBase
{
public:
  // Validate input file parameters
  static InputParameters validParams();

  // Constructor
  explicit FoamFixedValueBC(const InputParameters & parameters);
};
