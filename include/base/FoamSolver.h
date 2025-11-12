#pragma once

#include "fvMesh.H"
#include "scalar.H"
#include "solver.H"
#include "functionObject.H"

#include <Time.H>
#include <TimeState.H>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace Foam
{
namespace functionObjects
{
// Function object to tell OpenFOAM what MOOSE's dt is
class mooseDeltaT : public functionObject
{
private:
  const scalar & dt_;

public:
  TypeName("mooseDeltaT")

      mooseDeltaT(const word & name, const Time & runTime, const scalar & dt)
    : functionObject(name, runTime), dt_(dt)
  {
  }

  wordList fields() const { return wordList::null(); }

  bool executeAtStart() const { return false; }

  bool execute() { return true; }
  bool write() { return true; }
  scalar maxDeltaT() const { return dt_; }
};
}
}

namespace Hippo
{
class FoamSolver
{
public:
  explicit FoamSolver(Foam::solver * solver) : _solver(solver) {}

  // Run a timestep of the OpenFOAM solver.
  void run();
  // Return the number of faces in the given patch (boundary).
  std::size_t patchSize(int patch_id);
  // Set the solver's time step size.
  void setTimeDelta(double dt) { runTime().setDeltaTNoAdjust(dt); }
  // Get the current time from the solver.
  double currentTime() const { return _solver->time().userTimeValue(); }
  // Get the current time index from the solver.
  int currentTimeIdx() const
  {
    return _solver->time().findClosestTimeIndex(runTime().times(), currentTime());
  }
  // Set the solver to the given time.
  void setCurrentTime(double time) { runTime().setTime(time, runTime().timeIndex()); }
  // set the time index of the solver
  void setCurrentTimeIdx(int timeIdx) { runTime().setTime(runTime().userTimeValue(), timeIdx); }
  // Set the time at which the solver should terminate.
  void setEndTime(double time) { runTime().setEndTime(time); }
  // Run the presolve from MOOSE objects.
  void preSolve();
  // Provide access to the openfoam solver.
  Foam::solver & solver() { return *_solver; };
  // Calculate OpenFOAM's time step.
  Foam::scalar computeDeltaT();
  // check whether OpenFOAM has variable time step.
  bool isDeltaTAdjustable() const;
  // creates function object that tells OpenFOAM what MOOSE's
  // time step is.
  void appendDeltaTFunctionObject(const Foam::scalar & dt);
  // get the current deltaT.
  Foam::scalar getTimeDelta() const { return runTime().deltaTValue(); }
  // Get the path to the current time directory, this may or may not exist.
  fs::path currentTimePath() const { return fs::path(runTime().timePath()); }
  // Write the current OpenFOAM timestep to its time directory.
  void write() const { _solver->mesh.write(); }
  // Reset the solver to the given time.
  void readTime(const double time)
  {
    auto idx = _solver->time().findClosestTimeIndex(runTime().times(), time);
    runTime().setTime(time, idx);
    runTime().readModifiedObjects();
  }

private:
  Foam::solver * _solver = nullptr;

  Foam::Time & runTime() { return const_cast<Foam::Time &>(_solver->runTime); }
  const Foam::Time & runTime() const { return _solver->runTime; }
};

} // namespace Hippo
