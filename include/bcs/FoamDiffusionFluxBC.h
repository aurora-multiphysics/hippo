#pragma once

#include "FoamVariableBCBase.h"

/// Prescribe scalar diffusion flux, converted to a normal gradient using the diffusivity field.
class FoamDiffusionFluxBC : public FoamVariableBCBase
{
public:
  static InputParameters validParams();
  explicit FoamDiffusionFluxBC(const InputParameters & params);
};
