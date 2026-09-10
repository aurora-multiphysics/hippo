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
  void constructFoamPatch(Foam::label patch_id) override { constructFixedGradientPatch(patch_id); }

  // name of diffusivity coefficient used to divide flux
  std::string _diffusivity;
};
