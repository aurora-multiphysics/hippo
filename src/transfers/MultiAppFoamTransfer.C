#include "MultiAppFoamTransfer.h"
#include "FoamProblem.h"
#include "MooseError.h"

InputParameters
MultiAppFoamTransfer::validParams()
{
  return MultiAppTransfer::validParams();
}

MultiAppFoamTransfer::MultiAppFoamTransfer(const InputParameters & params)
  : MultiAppTransfer(params)
{
}

FoamProblem &
MultiAppFoamTransfer::foamProblem(FEProblemBase & problem) const
{
  auto * foam_problem = dynamic_cast<FoamProblem *>(&problem);
  if (!foam_problem)
    mooseError("Expected a FoamProblem at the OpenFOAM transfer endpoint.");

  return *foam_problem;
}
