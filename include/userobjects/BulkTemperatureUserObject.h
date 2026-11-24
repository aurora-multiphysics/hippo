#pragma once
#include "GeneralUserObject.h"

/// Base class for bulk temperature calculations for HTC.
class BulkTemperatureUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();
  BulkTemperatureUserObject(const InputParameters & params);

  void initialize() final {};
  void finalize() final {};

  Real spatialValue(const Point & /*p*/) const override = 0;
};
