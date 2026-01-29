#pragma once

#include "FoamSideIntegratedBase.h"
#include "InputParameters.h"
#include <functionObject.H>
#include <functionObjects/field/wallHeatFlux/wallHeatFlux.H>
#include <functionObjects/field/wallShearStress/wallShearStress.H>
#include <memory>

class FoamSideIntegratedFunctionObject : public FoamSideIntegratedBase
{
public:
  static InputParameters validParams();

  FoamSideIntegratedFunctionObject(const InputParameters & params);

protected:
  virtual Real integrateValue() override;

  /// Creates function objects to be executed by compute
  Foam::functionObject * createFunctionObject();

  std::unique_ptr<Foam::functionObject> _function_object;
};
