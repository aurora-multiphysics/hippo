#include "FoamProblem.h"
#include "FoamTimeStepper.h"
#include <pimpleSingleRegionControl.H>

#include <scalar.H>
#include <TimeStepper.h>
#include <Transient.h>
#include <solver.H>

registerMooseObject("hippoApp", FoamTimeStepper);

InputParameters
FoamTimeStepper::validParams()
{
  auto params = TimeStepper::validParams();
  return params;
}

FoamTimeStepper::FoamTimeStepper(InputParameters const & params)
  : TimeStepper(params), _foam_dt{}, _desired_dt{}, _moose_dt()
{
  auto problem = dynamic_cast<FoamProblem *>(&_app.feProblem());
  if (!problem)
  {
    mooseError("FoamTimeStepper expects to be used with FoamProblem");
  }
}

Real
FoamTimeStepper::computeDT()
{

  if (!_dt_adjustable)
    return _foam_dt;

  // Not ideal, but for MOOSE to get an accurate deltaT
  // preSolve must be called as this updates the BCs.
  solver().preSolve();

  // Tells the mooseDelta function object what the previous desired time
  // step was so it can work out whether there was a MOOSE induced cutback.
  _moose_dt->get().setOldDesiredDt(_desired_dt);

  // Ensure MOOSE gets OpenFOAM's time step unaffected by the mooseDeltaT
  // functionObject.
  _moose_dt->get().disable();

  // compute OpenFOAM's desired time step
  _desired_dt = solver().computeDeltaT();

  // reenable the function object
  _moose_dt->get().enable();

  return _desired_dt;
}

FoamProblem *
FoamTimeStepper::problem()
{
  auto problem = dynamic_cast<FoamProblem *>(&_app.feProblem());
  if (!problem)
  {
    mooseError("FoamTimeStepper expects to be used with FoamProblem");
  }
  return problem;
}

void
FoamTimeStepper::init()
{
  TimeStepper::init();

  // Apply start time from input file if it is present
  if (_executioner.isParamSetByUser("start_time"))
    solver().setCurrentTime(_time);

  // Apply end time from input file if it is present
  if (_executioner.isParamSetByUser("end_time"))
    solver().setEndTime(_end_time);

  if (_executioner.isParamSetByUser("dt"))
  {
    _foam_dt = _executioner.getParam<Real>("dt");
    _dt_adjustable = false;
    solver().setTimeDelta(_foam_dt);
    return;
  }
  // determine if OpenFOAM's time-step is adjustable in controlDict
  _dt_adjustable = solver().isDeltaTAdjustable();

  // The key idea is that runTime.functionObjects().maxDeltaT() in adjustDeltaT
  // loops over the function objects and chooses the minimum, so by having
  // a function Object that returns what MOOSE wants, OpenFOAM will use the
  // MOOSE time step if it is smaller than what OpenFOAM wants. As a result,
  // if MOOSE wants to add a synchronisation step OpenFOAM will also use it too.

  // create function object and append it to the solver's function object list
  _moose_dt = solver().appendDeltaTFunctionObject(_dt);
  _desired_dt = solver().getTimeDelta();
  if (!_dt_adjustable)
    _foam_dt = solver().getTimeDelta();
}
