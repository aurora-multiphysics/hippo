#pragma once

#include "FoamPostprocessorBCBase.h"
#include "InputParameters.h"

class FoamFixedGradientPostprocessorBC : public FoamPostprocessorBCBase
{
public:
  static InputParameters validParams();

  FoamFixedGradientPostprocessorBC(const InputParameters & params);

  virtual void imposeBoundaryCondition() override;

protected:
  // name of diffusivity coefficient used to divide flux
  std::string _diffusivity_coefficient;
};
