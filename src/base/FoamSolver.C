#include "FoamSolver.h"

#include <fixedGradientFvPatchFields.H>
#include <functionObjects/field/wallHeatFlux/wallHeatFlux.H>
#include <fvPatchField.H>
#include <pimpleSingleRegionControl.H>
#include <scalarField.H>
#include <volFieldsFwd.H>

#include <algorithm>
#include <cassert>
#include <iterator>

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

void FoamSolver::synchronizeAdaptiveTimes(double dt)
{
  double foam_dt  = runTime().deltaTValue();
  if (std::abs(dt - foam_dt) > 1e-6)
  {
    runTime().setDeltaTNoAdjust(dt);
  }
}

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
FoamSolver::run(double deltaT)
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
  synchronizeAdaptiveTimes(deltaT);
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
FoamSolver::appendPatchTemperatures(const int patch_id, std::vector<double> & foam_t)
{
  if (!_solver)
  {
    return 0;
  }
  auto & mesh = _solver->mesh;
  auto & temp = mesh.boundary()[patch_id].lookupPatchField<Foam::volScalarField, double>("T");
  std::copy(temp.begin(), temp.end(), std::back_inserter(foam_t));
  return temp.size();
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
FoamSolver::setPatchTemperatures(const int patch_id, const std::vector<double> & moose_t)
{
  if (!_solver)
  {
    return;
  }
  auto & mesh = _solver->mesh;
  auto & temp = const_cast<Foam::fvPatchField<double> &>(
      mesh.boundary()[patch_id].lookupPatchField<Foam::volScalarField, double>("T"));
  assert(moose_t.size() == static_cast<std::size_t>(temp.size()));
  std::copy(moose_t.begin(), moose_t.end(), temp.begin());
}

void
FoamSolver::setPatchNegativeHeatFlux(const int patch_id, std::vector<double> & negative_hf)
{
  if (!_solver)
  {
    return;
  }
  auto & mesh = _solver->mesh;
  auto & temp = const_cast<Foam::fvPatchField<double> &>(
      mesh.boundary()[patch_id].lookupPatchField<Foam::volScalarField, double>("T"));
  Foam::scalarField & temp_gradient(
      Foam::refCast<Foam::fixedGradientFvPatchScalarField>(temp).gradient());
  auto & thermal_conductivity =
      mesh.boundary()[patch_id].lookupPatchField<Foam::volScalarField, double>("kappa");
  assert(temp_gradient.size() == thermal_conductivity.size());
  for (auto i = 0; i < temp_gradient.size(); ++i)
  {
    temp_gradient[i] = negative_hf[i] / thermal_conductivity[i];
  }
}

std::size_t
FoamSolver::wallHeatFlux(const int patch_id, std::vector<double> & fill_vector)
{
  if (!_solver)
  {
    return 0;
  }

  static const Foam::word WALL_HEAT_FLUX = "wallHeatFlux";

  auto whf_dict =
      _solver->runTime.controlDict().lookupOrDefault(WALL_HEAT_FLUX, Foam::dictionary());
  auto patch = _solver->mesh.boundaryMesh()[patch_id];
  whf_dict.set("patches", Foam::wordList({patch.name()}));
  whf_dict.set("writeToFile", false);
  Foam::functionObjects::wallHeatFlux whf_func(WALL_HEAT_FLUX, _solver->runTime, whf_dict);
  whf_func.execute();

  auto wall_heat_flux = _solver->mesh.lookupObject<Foam::volScalarField>(WALL_HEAT_FLUX);
  auto & whf_boundary = wall_heat_flux.boundaryField()[patch.index()];
  for (const auto value : whf_boundary)
  {
    fill_vector.emplace_back(value);
  }
  return whf_boundary.size();
}

} // namespace Hippo
