#pragma once

#include "MooseError.h"
#include "scalar.H"
#include "solver.H"
#include "functionObject.H"

#include <Time.H>
#include <TimeState.H>

#include <optional>

namespace Foam
{
namespace functionObjects
{
/*
  The key idea is that runTime.functionObjects().maxDeltaT() in adjustDeltaT
  loops over the function objects and chooses the minimum
  - So by having a function Object that returns what MOOSE wants, OpenFOAM will use the
    MOOSE time step if it is smaller than what OpenFOAM wants.
  - As a result, if MOOSE wants to add a synchronisation step OpenFOAM will also use it too.
  - However, the MOOSE induced cutback can lead to a slow recovery of the timestep and more
    time steps being taken than necessary
  - So, after a cutback we modify DeltaTFactor to allow the next timestep to return to its
    value before the cutback.
*/
class mooseDeltaT : public functionObject
{
private:
  const scalar & _dt;
  std::optional<Foam::scalar> _old_desired_dt;
  const scalar _delta_t_factor;
  bool _enabled;

public:
  TypeName("mooseDeltaT")

      mooseDeltaT(const word & name, const Time & runTime, const scalar & dt)
    : functionObject(name, runTime),
      _dt(dt),
      _old_desired_dt(),
      _delta_t_factor(Foam::solver::deltaTFactor),
      _enabled(true)
  {
  }

  virtual wordList fields() const override { return wordList::null(); }

  virtual bool executeAtStart() const override { return false; }

  virtual bool execute() override { return true; }
  virtual bool write() override { return true; }
  void setOldDesiredDt(scalar desired_dt) { _old_desired_dt = desired_dt; }
  void enable() { _enabled = true; }
  void disable() { _enabled = false; }
  Foam::scalar calculateDeltaTFactor(const Foam::scalar time) const
  {
    if (!_old_desired_dt.has_value())
      mooseError("OldDesiredTimeStep must be set before the deltaTFactor is calculated");

    if (time != _old_desired_dt)
      return _delta_t_factor * _old_desired_dt.value() / time;
    else
      return _delta_t_factor;
  }
  virtual scalar maxDeltaT() const override
  {
    // If MOOSE altered the previous time step change the deltaTfactor to undo the MOOSE induced
    // cutback
    Foam::solver::deltaTFactor = calculateDeltaTFactor(time_.deltaTValue());

    // If we don't want MOOSE's timestep to be considered, we return the maximum value.
    if (!_enabled)
      return Foam::VGREAT;

    return _dt;
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
  // Set whether OpenFOAM can adjust the timestep
  void setDeltaTAdjustable(const bool adjustable);
  // get mooseDeltaT function object
  Foam::functionObjects::mooseDeltaT & getDeltaTFunctionObject();
  // get the current deltaT.
  Foam::scalar getTimeDelta() const { return runTime().deltaTValue(); }
  // creates function object that tells OpenFOAM what MOOSE's
  // time step is.
  void appendDeltaTFunctionObject(const Foam::scalar & dt);

private:
  Foam::solver * _solver = nullptr;

  Foam::Time & runTime() { return const_cast<Foam::Time &>(_solver->runTime); }
  const Foam::Time & runTime() const { return _solver->runTime; }
};

} // namespace Hippo
