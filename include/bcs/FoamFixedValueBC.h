#pragma once

#include "FoamBCBase.h"
#include "InputParameters.h"

class FoamFixedValueBC : public FoamBCBase
{
public:
  static InputParameters validParams();
  explicit FoamFixedValueBC(const InputParameters & parameters);
  virtual void imposeBoundaryCondition() override;
};
