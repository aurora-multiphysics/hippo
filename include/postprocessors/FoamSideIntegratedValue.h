#pragma once
#include "FoamSideIntegratedBase.h"

class FoamSideIntegratedValue : public FoamSideIntegratedBase
{
public:
  static InputParameters validParams();

  FoamSideIntegratedValue(const InputParameters & params);
};
