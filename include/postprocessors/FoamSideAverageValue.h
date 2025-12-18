#pragma once
#include "FoamSideIntegratedValue.h"
#include "InputParameters.h"

class FoamSideAverageValue : public FoamSideIntegratedValue
{
public:
  static InputParameters validParams() { return FoamSideIntegratedValue::validParams(); }

  FoamSideAverageValue(const InputParameters & params);

  virtual void compute() override;
};
