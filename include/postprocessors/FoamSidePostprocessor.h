#pragma once

#include "FoamPostprocessorBase.h"

class FoamSidePostprocessor : public FoamPostprocessorBase
{
public:
  static InputParameters validParams();

  FoamSidePostprocessor(const InputParameters & params);
};
