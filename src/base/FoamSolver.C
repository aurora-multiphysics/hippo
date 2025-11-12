#include "FoamSolver.h"

#include <fixedGradientFvPatchFields.H>
#include <functionObjects/field/wallHeatFlux/wallHeatFlux.H>
#include <fvPatchField.H>
#include <iostream>
#include <ostream>
#include <pimpleSingleRegionControl.H>
#include <scalarField.H>
#include <unistd.h>
#include <volFieldsFwd.H>

#include <algorithm>
#include <cassert>
#include <iterator>

namespace Foam
{
namespace functionObjects
{
defineTypeNameAndDebug(mooseDeltaT, 0);

}
}

namespace Hippo
{
namespace
{
/**
 * This was copied (with some minor adjustments) from
 * 'applications/solvers/foamRun/setDeltaT.C' OpenFOAM-12 revision
 * 9ec94dd57a8d98c3f3422ce9b2156a8b268bbda6.
 */
void
adjustDeltaT(Foam::Time & runTime, const Foam::solver & solver)
{
  // Update the time-step limited by the solver maxDeltaT
  if (runTime.controlDict().lookupOrDefault("adjustTimeStep", false) && solver.transient())
  {
    const Foam::scalar deltaT = std::min(solver.maxDeltaT(), runTime.functionObjects().maxDeltaT());

    if (deltaT < Foam::rootVGreat)
    {
      runTime.setDeltaT(std::min(Foam::solver::deltaTFactor * runTime.deltaTValue(), deltaT));
      std::cout << "deltaT = " << runTime.deltaTValue() << std::endl;
    }
  }
}

/**
 * This was copied (with some minor adjustments) from
 * 'applications/solvers/foamRun/setDeltaT.C' OpenFOAM-12 revision
 * 9ec94dd57a8d98c3f3422ce9b2156a8b268bbda6.
 */
void
setDeltaT(Foam::Time & runTime, const Foam::solver & solver)
{
  if (runTime.timeIndex() == 0 && runTime.controlDict().lookupOrDefault("adjustTimeStep", false) &&
      solver.transient())
  {
    const Foam::scalar deltaT = std::min(solver.maxDeltaT(), runTime.functionObjects().maxDeltaT());

    if (deltaT < Foam::rootVGreat)
    {
      runTime.setDeltaT(std::min(runTime.deltaTValue(), deltaT));
    }
  }
}
} // namespace

/**
 * This was copied from 'applications/solvers/foamRun/foamRun.C' OpenFOAM-12
 * revision 9ec94dd57a8d98c3f3422ce9b2156a8b268bbda6. Modifications made:
 *   - We already have a solver, mesh, and runtime, so the construction of them
 * was removed.
 *   - The outer pimple-loop was removed so we're only running one timestep at a
 * time.
 *   - Some changes to the logging.
 */
void
FoamSolver::run()
{
  if (_solver == nullptr)
  {
    return;
  }
  auto & time = runTime();
  auto & solver = *_solver;

  // Create the outer PIMPLE loop and control structure
  Foam::pimpleSingleRegionControl pimple(solver.pimple);

  // Set the initial time-step
  setDeltaT(time, solver);

  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

  // Update PIMPLE outer-loop parameters if changed
  pimple.read();
  solver.preSolve();

  // Adjust the time-step according to the solver maxDeltaT
  adjustDeltaT(time, solver);
  time++;

  // TODO: replace std::cout with MOOSE output or a dependency-injected stream.
  std::cout << "Time = " << time.userTimeName() << "\n" << std::endl;

  // PIMPLE corrector loop
  while (pimple.loop())
  {
    solver.moveMesh();
    solver.motionCorrector();
    solver.fvModels().correct();
    solver.prePredictor();
    solver.momentumPredictor();
    solver.thermophysicalPredictor();
    solver.pressureCorrector();
    solver.postCorrector();
  }

  solver.postSolve();

  time.write();

  std::cout << "ExecutionTime = " << time.elapsedCpuTime() << " s"
            << "  ClockTime = " << time.elapsedClockTime() << " s"
            << "\n"
            << std::endl;
}

std::size_t
FoamSolver::patchSize(int patch_id)
{
  if (!_solver)
  {
    return 0;
  }
  auto & mesh = _solver->mesh;
  return mesh.boundary()[patch_id].size();
}

void
FoamSolver::preSolve()
{
  _solver->pimple.readIfModified();
  _solver->preSolve();
}

Foam::scalar
FoamSolver::computeDeltaT()
{
  // This code has been adapted from OpenFOAM's adjustDeltaT to determine the time-step that
  // OpenFOAM will use on the next time step so MOOSE can predict it.
  Foam::scalar deltaT =
      std::min(_solver->maxDeltaT(), _solver->runTime.functionObjects().maxDeltaT());

  if (deltaT < Foam::rootVGreat)
    return std::min(Foam::solver::deltaTFactor * _solver->runTime.deltaTValue(), deltaT);
  return _solver->runTime.deltaTValue();
}

bool
FoamSolver::isDeltaTAdjustable() const
{
  return _solver->runTime.controlDict().lookupOrDefault("adjustTimeStep", false);
}

void
FoamSolver::appendDeltaTFunctionObject(const Foam::scalar & dt)
{
  runTime().functionObjects().append(
      new Foam::functionObjects::mooseDeltaT("Moose time step", runTime(), dt));
}
} // namespace Hippo
