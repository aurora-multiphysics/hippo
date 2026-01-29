#pragma once

#include "FoamSideIntegratedFunctionObject.h"
#include "InputParameters.h"

class FoamSideAverageFunctionObject : public FoamSideIntegratedFunctionObject
{
public:
  static InputParameters validParams() { return FoamSideIntegratedFunctionObject::validParams(); }

  FoamSideAverageFunctionObject(const InputParameters & params)
    : FoamSideIntegratedFunctionObject(params)
  {
  }

  virtual void compute() override;
};
