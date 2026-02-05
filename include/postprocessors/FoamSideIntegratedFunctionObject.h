#pragma once

#include "FoamSideIntegratedBase.h"
#include "InputParameters.h"

#include <memory>

class FoamSideIntegratedFunctionObject : public FoamSideIntegratedBase
{
public:
  static InputParameters validParams();

  FoamSideIntegratedFunctionObject(const InputParameters & params);

  virtual void compute() override;

protected:
  /// Creates function objects to be executed by compute
  std::unique_ptr<Foam::functionObject> createFunctionObject(const std::string & fo_name);

  std::unique_ptr<Foam::functionObject> _function_object;
};
