#include "FoamBCBase.h"
#include "MooseObject.h"
#include "Registry.h"

registerMooseObject("hippoApp", FoamBCBase);

InputParameters
FoamBCBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.registerSystemAttributeName("FoamBC");
  params.registerBase("FoamBC");

  return params;
}

FoamBCBase::FoamBCBase(const InputParameters & params) : MooseObject(params) {}
