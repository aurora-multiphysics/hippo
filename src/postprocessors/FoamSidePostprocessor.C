#include "BlockRestrictable.h"
#include "FoamPostprocessorBase.h"
#include "FoamSidePostprocessor.h"
#include "InputParameters.h"
#include "MooseTypes.h"

InputParameters
FoamSidePostprocessor::validParams()
{
  auto params = FoamPostprocessorBase::validParams();
  params.addRequiredParam<SubdomainName>("boundary", "Boundary where this postprocessor applies");

  // Remove once changedto BoundaryRestrictable
  params.addPrivateParam("_dual_restrictable", false);
  params.addPrivateParam<std::vector<SubdomainName>>("block");
  return params;
}

FoamSidePostprocessor::FoamSidePostprocessor(const InputParameters & params)
  : FoamPostprocessorBase(params), BlockRestrictable(this)
{
}
