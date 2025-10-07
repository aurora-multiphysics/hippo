#include "FoamPostprocessorBase.h"
#include "FoamProblem.h"

InputParameters
FoamPostprocessorBase::validParams()
{
  auto params = UserObject::validParams();
  params += Postprocessor::validParams();
  return params;
}

FoamPostprocessorBase::FoamPostprocessorBase(const InputParameters & params)
  : UserObject(params), Postprocessor(this), _mesh(nullptr)
{
  FoamProblem * problem = dynamic_cast<FoamProblem *>(&getSubProblem());
  if (!problem)
    mooseError("FoamPostprocessors can only be used with FoamProblem");

  _mesh = &problem->mesh();
}
