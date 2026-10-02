#pragma once

#include "FoamPostprocessorBCBase.h"
#include "InputParameters.h"

class FoamFixedGradientPostprocessorBC : public FoamPostprocessorBCBase
{
public:
  static InputParameters validParams();

  FoamFixedGradientPostprocessorBC(const InputParameters & params);

  // impose boundary condition
  virtual void imposeBoundaryCondition(bool initialisation = false) override;

protected:
  void constructFoamPatch(Foam::label patch_id) override { constructFixedGradientPatch(patch_id); }
};
