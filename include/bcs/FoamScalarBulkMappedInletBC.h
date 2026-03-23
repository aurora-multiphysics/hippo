#pragma once
#include "FoamMappedInletBCBase.h"
#include "MooseEnum.h"

class FoamScalarBulkMappedInletBC : public FoamMappedInletBCBase
{
public:
  static InputParameters validParams();

  FoamScalarBulkMappedInletBC(const InputParameters & params);

  virtual void imposeBoundaryCondition() override;

protected:
  MooseEnum _scale_method;

  template <typename T>
  T applyScaleMethod(T & var, const Real bulk_ref, const Real bulk);
};
