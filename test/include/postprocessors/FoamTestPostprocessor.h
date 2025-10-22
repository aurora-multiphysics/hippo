#pragma once

#include "FoamSidePostprocessor.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "UserObject.h"

class FoamTestPostprocessor : public FoamSidePostprocessor
{
public:
  static InputParameters validParams();

  FoamTestPostprocessor(const InputParameters & params);

  virtual void execute() override;

  virtual void initialize() override;

  virtual void finalize() override;

  virtual void threadJoin(const UserObject & uo) override;

  virtual PostprocessorValue getValue() const override;

protected:
  Real _value;
};
