#pragma once
#include "FoamMappedInletBCBase.h"

class FoamScalarBulkMappedInletBC : public FoamMappedInletBCBase
{
public:
  static InputParameters validParams();

  FoamScalarBulkMappedInletBC(const InputParameters & params);

  virtual void imposeBoundaryCondition() override;
};
