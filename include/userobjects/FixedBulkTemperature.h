#pragma once

#include "BulkTemperatureUserObject.h"
#include "InputParameters.h"

/// UserObject that applies a fixed reference temperature for HTC
/// calculations
class FixedBulkTemperature : public BulkTemperatureUserObject
{
public:
  static InputParameters validParams();
  FixedBulkTemperature(const InputParameters & params);

  void execute() override {}

  Real spatialValue(const Point & point) const override;

private:
  const Real _T_bulk;
};
