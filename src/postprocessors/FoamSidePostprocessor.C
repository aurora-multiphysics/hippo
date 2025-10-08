#include "ElementUserObject.h"
#include "FoamSidePostprocessor.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "FoamProblem.h"
#include "Postprocessor.h"

InputParameters
FoamSidePostprocessor::validParams()
{
  auto params = ElementUserObject::validParams();
  params += Postprocessor::validParams();

  // Remove once changedto BoundaryRestrictable
  params.addPrivateParam("_dual_restrictable", false);
  return params;
}

FoamSidePostprocessor::FoamSidePostprocessor(const InputParameters & params)
  : ElementUserObject(params), Postprocessor(this)
{
  FoamProblem * problem = dynamic_cast<FoamProblem *>(&getSubProblem());
  if (!problem)
    mooseError("FoamPostprocessors can only be used with FoamProblem");
}
