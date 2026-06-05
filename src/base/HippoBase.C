#include "FoamProblem.h"
#include "HippoBase.h"
#include "InputParameters.h"
#include "MooseObject.h"
#include "OutputInterface.h"

InputParameters
HippoBase::validParams()
{
  return MooseObject::validParams();
}

HippoBase::HippoBase(const InputParameters & params) : MooseObject(params) {}

FoamProblem &
HippoBase::getFoamProblem() const
{
  auto * problem = dynamic_cast<FoamProblem *>(&getMooseApp().feProblem());
  if (!problem)
    mooseError("This object can only be used with FoamProblem");

  return *problem;
}
