#include "BlockRestrictable.h"
#include "FoamSidePostprocessor.h"
#include "InputParameters.h"
#include "MooseTypes.h"

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
}
