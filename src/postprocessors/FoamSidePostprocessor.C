#include "BlockRestrictable.h"
#include "FoamSidePostprocessor.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "FoamProblem.h"

InputParameters
FoamSidePostprocessor::validParams()
{
  auto params = FoamPostprocessorBase::validParams();
  params += BlockRestrictable::validParams();

  return params;
}

FoamSidePostprocessor::FoamSidePostprocessor(const InputParameters & params)
  : FoamPostprocessorBase(params), _volume(0.)
{
  FoamProblem * problem = dynamic_cast<FoamProblem *>(&getSubProblem());
  if (!problem)
    mooseError("FoamPostprocessors can only be used with FoamProblem");
}
