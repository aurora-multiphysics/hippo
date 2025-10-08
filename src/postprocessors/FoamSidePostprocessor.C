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

  return params;
}

FoamSidePostprocessor::FoamSidePostprocessor(const InputParameters & params)
  : ElementUserObject(params), Postprocessor(this), _volume(0.)
{
  FoamProblem * problem = dynamic_cast<FoamProblem *>(&getSubProblem());
  if (!problem)
    mooseError("FoamPostprocessors can only be used with FoamProblem");
}
