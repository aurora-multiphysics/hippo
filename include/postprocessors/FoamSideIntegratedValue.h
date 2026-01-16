#pragma once
#include "FoamSidePostprocessor.h"
#include <functionObjects/field/wallHeatFlux/wallHeatFlux.H>
#include <functionObjects/field/wallShearStress/wallShearStress.H>

static MooseEnum _pp_function_objects("wallHeatFlux wallShearStress");

class FoamSideIntegratedValue : public FoamSidePostprocessor
{
public:
  static InputParameters validParams();

  FoamSideIntegratedValue(const InputParameters & params);

  virtual PostprocessorValue getValue() const override;

  virtual void compute() override;

protected:
  /// Creates function objects to be executed by compute
  void createFunctionObject();

  Real _value;

  std::string _foam_variable;

  bool _is_vector;

  Foam::functionObject * _function_object;
};
