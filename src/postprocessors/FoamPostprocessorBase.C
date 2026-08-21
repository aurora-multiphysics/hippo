#include "FoamMesh.h"
#include "FoamPostprocessorBase.h"
#include "HippoInterface.h"
#include "InputParameters.h"
#include "Postprocessor.h"
#include "ElementUserObject.h"
#include "FoamProblem.h"

InputParameters
FoamPostprocessorBase::validParams()
{
  auto params = ElementUserObject::validParams();
  params += Postprocessor::validParams();
  return params;
}

FoamPostprocessorBase::FoamPostprocessorBase(const InputParameters & params)
  : ElementUserObject(params), Postprocessor(this), HippoInterface(this)
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
