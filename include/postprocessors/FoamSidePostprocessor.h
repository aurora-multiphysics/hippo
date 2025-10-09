#pragma once

#include "FoamPostprocessorBase.h"

// When FoamMesh is updated  use BoundaryRestrictable when Foammesh is refactored
class FoamSidePostprocessor : public FoamPostprocessorBase
{
public:
  static InputParameters validParams();

  FoamSidePostprocessor(const InputParameters & params);

protected:
  Real _volume;
};
