#pragma once

#include "GeneralUserObject.h"
#include "InputParameters.h"

class FixedBulkTemperature : public GeneralUserObject
{
public:
  static InputParameters validParams();
  FixedBulkTemperature(const InputParameters & params);

  void execute() override {}
  void initialize() override {}
  void finalize() override {};

  Real spatialValue(const Point & point) const override;

private:
  const Real _T_bulk;
};
