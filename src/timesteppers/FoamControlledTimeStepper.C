#include "FoamProblem.h"
#include "FoamControlledTimeStepper.h"
#include "functionObject.H"
#include <pimpleSingleRegionControl.H>

#include <scalar.H>
#include <TimeStepper.h>
#include <Transient.h>

registerMooseObject("hippoApp", FoamControlledTimeStepper);

namespace Foam
{
namespace functionObjects
{
defineTypeNameAndDebug(mooseDeltaT, 0);

}
}

InputParameters
FoamControlledTimeStepper::validParams()
{
  auto params = TimeStepper::validParams();
  return params;
}

FoamControlledTimeStepper::FoamControlledTimeStepper(InputParameters const & params)
  : TimeStepper(params)
{
  auto problem = dynamic_cast<FoamProblem *>(&_app.feProblem());
  if (!problem)
  {
    mooseError("FoamTimeStepper expects to be used with FoamProblem");
  }
}

Real
FoamControlledTimeStepper::computeDT()
{

  if (!_dt_adjustable)
    return _foam_initial_dt;

  // Not ideal, but for MOOSE to get an accurate deltaT
  // preSolve must be called as this updates the BCs.
  solver().preSolve();

  // Ensure MOOSE gets OpenFOAM's time step unaffected by the mooseDeltaT
  // functionObject.
  Real dt_tmp = _dt;
  _dt = Foam::rootVGreat;

  // compute OpenFOAM's desired time step
  Real deltaT = solver().computeDeltaT();

  // reset MOOSE's time step and return
  _dt = dt_tmp;
  return deltaT;
}

FoamProblem *
FoamControlledTimeStepper::problem()
{
  auto problem = dynamic_cast<FoamProblem *>(&_app.feProblem());
  if (!problem)
  {
    mooseError("FoamTimeStepper expects to be used with FoamProblem");
  }
  return problem;
}

void
FoamControlledTimeStepper::init()
{
  TimeStepper::init();

  Foam::solver & foam_solver{solver().solver()};

  // determine if OpenFOAM's time-step is adjustable in controlDict
  _dt_adjustable = foam_solver.runTime.controlDict().lookupOrDefault("adjustTimeStep", false);

  // Add functionObject that tells OpenFOAM what MOOSE's time step is.
  // If MOOSE inserts a timestep then functionObjects.maxDeltaT() will return
  // MOOSE's timestep
  Foam::Time & runtime = const_cast<Foam::Time &>(foam_solver.runTime);

  runtime.functionObjects().append(
      new Foam::functionObjects::mooseDeltaT("Moose time step", foam_solver.runTime, _dt));

  if (!_dt_adjustable)
    _foam_initial_dt = foam_solver.runTime.deltaTValue();
}
