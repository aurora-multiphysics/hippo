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
