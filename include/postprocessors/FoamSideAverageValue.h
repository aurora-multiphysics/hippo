#pragma once
#include "FoamSideIntegratedValue.h"
#include "InputParameters.h"
#include <functionObjects/field/wallHeatFlux/wallHeatFlux.H>
#include <functionObjects/field/wallShearStress/wallShearStress.H>

class FoamSideAverageValue : public FoamSideIntegratedValue
{
public:
  static InputParameters validParams() { return FoamSideIntegratedValue::validParams(); }

  FoamSideAverageValue(const InputParameters & params);
  virtual void compute() override;
};
