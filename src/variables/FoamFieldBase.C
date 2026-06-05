#include "FoamFieldBase.h"
#include "HippoBase.h"

InputParameters
FoamFieldBase::validParams()
{
  auto params = HippoBase::validParams();

  params.registerBase("FoamVariable");
  params.registerSystemAttributeName("FoamVariable");
  return params;
}

FoamFieldBase::FoamFieldBase(const InputParameters & params) : HippoBase(params) {}
