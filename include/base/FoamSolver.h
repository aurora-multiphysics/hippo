#pragma once

#include "solver.H"
#include "functionObject.H"

#include <vector>

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

  // Append temperature values from the OpenFOAM patch (boundary) to the end of
  // the given vector.
  std::size_t appendPatchTemperatures(int patch_id, std::vector<double> & foam_t);
  // Return the number of faces in the given patch (boundary).
  std::size_t patchSize(int patch_id);
  // Set the temperature values of the OpenFOAM patch (boundary).
  void setPatchTemperatures(int patch_id, const std::vector<double> & moose_t);
  // Set the negative heat flux values on the OpenFOAM patch (boundary).
  void setPatchNegativeHeatFlux(int patch_id, std::vector<double> & negative_hf);
  // Get or calculate the wall heat flux for the given patch.
  std::size_t wallHeatFlux(int patch_id, std::vector<double> & fill_vector);
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
  void appendDeltaTFunctionObject(const Foam::scalar & dt);
  // get the current deltaT.
  Foam::scalar getTimeDelta() { return runTime().deltaTValue(); };

private:
  Foam::solver * _solver = nullptr;

  Foam::Time & runTime() { return const_cast<Foam::Time &>(_solver->runTime); }
};

} // namespace Hippo
