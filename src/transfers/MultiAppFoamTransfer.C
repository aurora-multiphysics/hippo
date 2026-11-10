#include "HippoInterface.h"
#include "MultiAppFoamTransfer.h"
#include "MultiAppTransfer.h"
#include "FoamProblem.h"
#include "DisplacedProblem.h"
#include "libmesh/equation_systems.h"

InputParameters
MultiAppFoamTransfer::validParams()
{
  return MultiAppTransfer::validParams();
}

MultiAppFoamTransfer::MultiAppFoamTransfer(const InputParameters & params)
  : MultiAppTransfer(params)
{
}

libMesh::EquationSystems &
MultiAppFoamTransfer::getlibMeshEquationSystem(FEProblemBase & problem, bool use_displaced) const
{
  if (use_displaced)
  {
    if (!problem.getDisplacedProblem())
      mooseError("No displaced problem to provide a displaced equation system");

    return problem.getDisplacedProblem()->es();
  }
  else
    return problem.es();
}

FoamProblem &
MultiAppFoamTransfer::foamProblem(FEProblemBase & problem) const
{
  auto * foam_problem = dynamic_cast<FoamProblem *>(&problem);
  if (!foam_problem)
    mooseError("Expected a FoamProblem at the OpenFOAM transfer endpoint.");

  return *foam_problem;
}
