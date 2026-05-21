#include "FoamProblem.h"
#include "HippoBase.h"
#include "InputParameters.h"
#include "MooseObject.h"
#include "OutputInterface.h"

HippoBase::HippoBase(const MooseObject * moose_object)
  : _foam_problem(extractFoamProblemPtr(moose_object))
{
}

FoamProblem *
HippoBase::extractFoamProblemPtr(const MooseObject * moose_object)
{
  const InputParameters & params = moose_object->parameters();
  auto * problem = params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base");
  auto * foam_problem = dynamic_cast<FoamProblem *>(problem);
  if (!foam_problem)
    mooseError("This object can only be used with FoamProblem");
  return foam_problem;
}

FoamProblem &
HippoBase::getFoamProblem() const
{
  return *_foam_problem;
}
