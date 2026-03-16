#pragma once

#include "scalar.H"
#include "solver.H"
#include "functionObject.H"

#include <Time.H>
#include <TimeState.H>

namespace Foam
{
namespace functionObjects
{
// Function object to tell OpenFOAM what MOOSE's dt is
class mooseDeltaT : public functionObject
{
private:
  const scalar & dt_;
  scalar oldDesiredDt_;
  const scalar deltaTFactor_;
  bool enabled_;

public:
  TypeName("mooseDeltaT")

      mooseDeltaT(const word & name, const Time & runTime, const scalar & dt)
    : functionObject(name, runTime),
      dt_(dt),
      oldDesiredDt_(time_.deltaTValue()),
      deltaTFactor_(Foam::solver::deltaTFactor),
      enabled_(true)
  {
  }

  wordList fields() const { return wordList::null(); }

  bool executeAtStart() const { return false; }

  bool execute() { return true; }
  bool write() { return true; }
  void setOldDesiredDt(scalar desired_dt) { oldDesiredDt_ = desired_dt; }
  void enable() { enabled_ = true; }
  void disable() { enabled_ = false; }
  scalar maxDeltaT() const
  {
    // If we don't want MOOSE's timestep to be considered, we return the maximum value.
    if (!enabled_)
      return Foam::VGREAT;

    // If MOOSE altered the previous time step change the deltaTfactor to undo the MOOSE induced
    // cutback
    if (time_.deltaTValue() != oldDesiredDt_)
      Foam::solver::deltaTFactor = deltaTFactor_ * oldDesiredDt_ / time_.deltaTValue();
    else
      Foam::solver::deltaTFactor = deltaTFactor_;
    return dt_;
  }
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
  // Set the solver to the given time.
  void setCurrentTime(double time) { runTime().setTime(time, runTime().timeIndex()); }
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
  Foam::functionObjects::mooseDeltaT & appendDeltaTFunctionObject(const Foam::scalar & dt);
  // get the current deltaT.
  Foam::scalar getTimeDelta() const { return runTime().deltaTValue(); }

private:
  Foam::solver * _solver = nullptr;

  Foam::Time & runTime() { return const_cast<Foam::Time &>(_solver->runTime); }
  const Foam::Time & runTime() const { return _solver->runTime; }
};

} // namespace Hippo
