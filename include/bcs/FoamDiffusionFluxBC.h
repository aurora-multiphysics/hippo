#pragma once

#include "FoamVariableBCBase.h"

class FoamDiffusionFluxBC : public FoamVariableBCBase
{
public:
  static InputParameters validParams();
  explicit FoamDiffusionFluxBC(const InputParameters & params);

  // Impose boundary conditions (to be called from FoamProblem class)
  virtual void imposeBoundaryCondition() override;

protected:
  // diffusivity name for flux condition
  const std::string _diffusivity;
};
