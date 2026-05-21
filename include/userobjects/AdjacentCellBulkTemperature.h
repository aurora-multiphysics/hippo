#pragma once

#include "GeneralUserObject.h"
#include "HippoBase.h"
#include "InputParameters.h"
#include <fvPatch.H>
#include <scalarField.H>

class AdjacentCellBulkTemperature : public GeneralUserObject, public HippoBase
{
public:
  static InputParameters validParams();
  AdjacentCellBulkTemperature(const InputParameters & params);

  void execute() override {}
  void initialize() override {}
  void finalize() override {};

  Real spatialValue(const Point & point) const override;

private:
  const Foam::fvPatch & getFoamPatch(const std::string & boundary);
  const Foam::fvPatch & _foam_patch;
};
