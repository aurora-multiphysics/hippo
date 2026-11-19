#include "fvCFD_moose.h"
#include "FoamFieldBase.h"
#include "MooseVariableBase.h"

InputParameters
FoamFieldBase::validParams()
{
  auto params = HippoObject::validParams();

  // Get desired parameters from Variable objects
  params.transferParam<std::vector<Real>>(MooseVariableBase::validParams(), "initial_condition");

  params.registerBase("FoamVariable");
  params.registerSystemAttributeName("FoamVariable");
  return params;
}

FoamFieldBase::FoamFieldBase(const InputParameters & params) : HippoObject(params) {}

bool
FoamFieldBase::mirrorEnabled(const InputParameters & params)
{
  const bool scalar = params.get<bool>("_scalar_foam_variable");
  const bool mirror = params.isParamValid("create_aux_variable")
                          ? params.get<bool>("create_aux_variable")
                          : scalar && params.isParamValid("foam_variable");
  if (mirror && !scalar)
    mooseError("Vector boundary mirroring is not supported; set create_aux_variable = false.");
  return mirror;
}
