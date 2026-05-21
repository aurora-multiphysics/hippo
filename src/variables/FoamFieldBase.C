#include "FoamFieldBase.h"
#include "HippoBase.h"
#include "MooseObject.h"

InputParameters
FoamFieldBase::validParams()
{
  auto params = MooseObject::validParams();

  params.registerBase("FoamVariable");
  params.registerSystemAttributeName("FoamVariable");
  return params;
}

FoamFieldBase::FoamFieldBase(const InputParameters & params) : MooseObject(params), HippoBase(this)
{
}
