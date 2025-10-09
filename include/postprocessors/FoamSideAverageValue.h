#pragma once
#include "FoamSidePostprocessor.h"

class FoamSideAverageValue : public FoamSidePostprocessor
{
public:
  static InputParameters validParams();

  FoamSideAverageValue(const InputParameters & params);

  virtual PostprocessorValue getValue() const override;

  virtual void compute() override;

protected:
  Real _value;

  std::string _foam_scalar;
};
