#pragma once
#include "FoamSidePostprocessor.h"

class FoamSideAdvectiveFluxIntegral : public FoamSidePostprocessor
{
public:
  static InputParameters validParams();

  FoamSideAdvectiveFluxIntegral(const InputParameters & params);

  virtual PostprocessorValue getValue() const override;

  virtual void compute() override;

protected:
  Real _value;

  std::string _foam_scalar;
  std::string _advection_velocity;
};
