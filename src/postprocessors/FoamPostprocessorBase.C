#include "FoamPostprocessorBase.h"
#include "InputParameters.h"
#include "Postprocessor.h"
#include "ElementUserObject.h"

InputParameters
FoamPostprocessorBase::validParams()
{

  auto params = ElementUserObject::validParams();
  params += Postprocessor::validParams();
  return params;
}

FoamPostprocessorBase::FoamPostprocessorBase(const InputParameters & params)
  : ElementUserObject(params), Postprocessor(this)
{
}

void
FoamPostprocessorBase::initialize()
{
}

void
FoamPostprocessorBase::execute()
{
}

void
FoamPostprocessorBase::finalize()
{
}

void
FoamPostprocessorBase::threadJoin(const UserObject & uo)
{
  (void)uo;
}
