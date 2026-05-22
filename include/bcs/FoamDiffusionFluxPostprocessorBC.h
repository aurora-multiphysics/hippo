#pragma once

#include "FoamPostprocessorBCBase.h"
#include "InputParameters.h"

class FoamDiffusionFluxPostprocessorBC : public FoamPostprocessorBCBase
{
public:
  static InputParameters validParams();

  FoamDiffusionFluxPostprocessorBC(const InputParameters & params);

  // impose boundary condition
  virtual void imposeBoundaryCondition() override;

protected:
  // name of diffusivity coefficient used to divide flux
  std::string _diffusivity;
};
