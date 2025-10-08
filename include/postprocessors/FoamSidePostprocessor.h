#pragma once

#include "ElementUserObject.h"
#include "InputParameters.h"

// When FoamMesh is updated  use BoundaryRestrictable when Foammesh is refactored
class FoamSidePostprocessor : public ElementUserObject, public Postprocessor
{
public:
  static InputParameters validParams();

  FoamSidePostprocessor(const InputParameters & params);

protected:
  Real _volume;
};
