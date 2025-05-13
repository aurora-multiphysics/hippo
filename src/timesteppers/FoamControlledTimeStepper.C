#include "FoamProblem.h"
#include "FoamControlledTimeStepper.h"
#include <pimpleSingleRegionControl.H>

#include <scalar.H>
#include <TimeStepper.h>
#include <Transient.h>

registerMooseObject("hippoApp", FoamControlledTimeStepper);

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

  // determine if OpenFOAM's time-step is adjustable in controlDict
  _dt_adjustable = solver().isDeltaTAdjustable();

  // The key idea is that runTime.functionObjects().maxDeltaT() in adjustDeltaT
  // loops over the function objects and chooses the minimum, so by having
  // a function Object that returns what MOOSE wants, OpenFOAM will use the
  // MOOSE time step if it is smaller than what OpenFOAM wants. As a result,
  // if MOOSE wants to add a synchronisation step OpenFOAM will also use it too.

  // create function object and append it to the solver's function object list
  solver().appendDeltaTFunctionObject(_dt);

  if (!_dt_adjustable)
    _foam_initial_dt = solver().getTimeDelta();
}
