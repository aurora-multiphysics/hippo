#include "FoamSolver.H"

#include <pimpleSingleRegionControl.H>

namespace Hippo
{
namespace
{
/**
 * This was copied (with some minor adjustments) from 'applications/solvers/foamRun/setDeltaT.C'
 * OpenFOAM-12 revision 9ec94dd57a8d98c3f3422ce9b2156a8b268bbda6.
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
 * This was copied (with some minor adjustments) from 'applications/solvers/foamRun/setDeltaT.C'
 * OpenFOAM-12 revision 9ec94dd57a8d98c3f3422ce9b2156a8b268bbda6.
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
}

/**
 * This was copied from 'applications/solvers/foamRun/foamRun.C' OpenFOAM-12 revision
 * 9ec94dd57a8d98c3f3422ce9b2156a8b268bbda6.
 * Modifications made:
 *   - We already have a solver, mesh, and runtime, so the construction of them was removed.
 *   - The outer pimple-loop was removed so we're only running one timestep at a time.
 *   - Some changes to the logging.
 */
void
FoamSolver::run()
{
  auto & runTime = const_cast<Foam::Time &>(_solver->runTime);
  auto & solver = *_solver;

  // Create the outer PIMPLE loop and control structure
  Foam::pimpleSingleRegionControl pimple(solver.pimple);

  // Set the initial time-step
  setDeltaT(runTime, solver);

  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

  // Update PIMPLE outer-loop parameters if changed
  pimple.read();

  solver.preSolve();

  // Adjust the time-step according to the solver maxDeltaT
  adjustDeltaT(runTime, solver);

  runTime++;

  // TODO: replace std::cout with MOOSE output or a dependency-injected stream.
  std::cout << "Time = " << runTime.userTimeName() << "\n" << std::endl;

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

  runTime.write();

  std::cout << "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
            << "  ClockTime = " << runTime.elapsedClockTime() << " s"
            << "\n"
            << std::endl;
}

}
