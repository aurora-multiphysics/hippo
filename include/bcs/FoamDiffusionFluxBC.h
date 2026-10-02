#pragma once

#include "FoamVariableBCBase.h"

class FoamDiffusionFluxBC : public FoamVariableBCBase
{
public:
  static InputParameters validParams();
  explicit FoamDiffusionFluxBC(const InputParameters & params);

  // Impose boundary conditions (to be called from FoamProblem class)
  virtual void imposeBoundaryCondition(bool initialisation = false) override;

protected:
  void constructFoamPatch(Foam::label patch_id) override { constructFixedGradientPatch(patch_id); }

  // diffusivity name for flux condition
  const std::string _diffusivity;
};
