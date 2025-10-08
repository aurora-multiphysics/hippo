#include "FoamPostprocessorBase.h"
#include "FoamProblem.h"
#include "InputParameters.h"
#include "MooseTypes.h"

InputParameters
FoamPostprocessorBase::validParams()
{
  auto params = UserObject::validParams();
  params += Postprocessor::validParams();
  return params;
}

FoamPostprocessorBase::FoamPostprocessorBase(const InputParameters & params)
  : UserObject(params),
    Postprocessor(this),
    _mesh(nullptr),
    _boundary(getParam<SubdomainName>("boundary"))
{
  FoamProblem * problem = dynamic_cast<FoamProblem *>(&getSubProblem());
  if (!problem)
    mooseError("FoamPostprocessors can only be used with FoamProblem");

  _mesh = &problem->mesh();

  // Remove once updated to use BoundaryRestrictable
  const_cast<InputParameters &>(params).set<std::vector<SubdomainName>>("block") = {
      params.get<SubdomainName>("boundary")};
}
