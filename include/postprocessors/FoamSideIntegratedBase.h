#pragma once
#include "FoamSidePostprocessor.h"

class FoamSideIntegratedBase : public FoamSidePostprocessor
{
public:
  static InputParameters validParams();

  FoamSideIntegratedBase(const InputParameters & params);

  virtual PostprocessorValue getValue() const override;

protected:
  Real integrateValue();

  Real _value;

  std::string _foam_variable;

  bool _is_vector;
};
