#pragma once

#include "UserObject.h"
#include "Postprocessor.h"

#include "FoamMesh.h"

class FoamPostprocessorBase : public UserObject, public Postprocessor
{
public:
  static InputParameters validParams();

  FoamPostprocessorBase(const InputParameters & params);

protected:
  FoamMesh * _mesh;
};
