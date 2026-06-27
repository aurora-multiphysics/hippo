#include "FoamProblem.h"
#include "HippoInterface.h"
#include "InputParameters.h"
#include "MooseObject.h"
#include "OutputInterface.h"

HippoInterface::HippoInterface(const MooseObject * moose_object)
  : _foam_problem(extractFoamProblem(moose_object)), _foam_fvmesh(_foam_problem.mesh().fvMesh())
{
}

FoamProblem &
HippoInterface::extractFoamProblem(const MooseObject * moose_object)
{
  const InputParameters & params = moose_object->parameters();
  auto * problem = params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base");
  auto * foam_problem = dynamic_cast<FoamProblem *>(problem);
  if (!foam_problem)
    mooseError("This object can only be used with FoamProblem");
  return *foam_problem;
}
