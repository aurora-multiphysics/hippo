#pragma once
#include "FoamSidePostprocessor.h"

class FoamSideAverageValue : public FoamSidePostprocessor
{
public:
  static InputParameters validParams();

  FoamSideAverageValue(const InputParameters & params);

  virtual void execute() override;

  virtual void initialize() override;

  virtual void finalize() override;

  virtual void threadJoin(const UserObject & uo) override;

  virtual PostprocessorValue getValue() const override;

protected:
  Real _value;

  std::string _foam_scalar;
};
