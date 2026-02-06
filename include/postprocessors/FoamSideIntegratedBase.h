#pragma once
#include "FoamSidePostprocessor.h"

class FoamSideIntegratedBase : public FoamSidePostprocessor
{
public:
  static InputParameters validParams();

  FoamSideIntegratedBase(const InputParameters & params);

  virtual PostprocessorValue getValue() const override;

protected:
  virtual Real integrateValue(const std::string & variable);

  Real getArea();

  Real _value;
};
