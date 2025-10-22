#include "FoamMesh.h"
#include "FoamPostprocessorBase.h"
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
  : ElementUserObject(params), Postprocessor(this), _foam_mesh(nullptr)
{
  FoamProblem * problem = dynamic_cast<FoamProblem *>(&getSubProblem());
  if (!problem)
    mooseError("Foam-based Postprocessors can only be used with FoamProblem");

  _foam_mesh = &problem->mesh().fvMesh();
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
