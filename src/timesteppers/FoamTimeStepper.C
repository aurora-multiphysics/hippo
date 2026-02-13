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
  : TimeStepper(params), _foam_dt{}, _desired_dt{}, _moose_dt(nullptr)
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

  // Tells mooseDelta function object to prevent the time step being cutback at the next step if the
  // previous one was cut back by MOOSE. THis again is pretty horrific but needs must.
  if (solver().getTimeDelta() != _desired_dt)
    _moose_dt->setAltered(true);
  else
    _moose_dt->setAltered(false);

  // Ensure MOOSE gets OpenFOAM's time step unaffected by the mooseDeltaT
  // functionObject.
  Real dt_tmp = _dt;
  _dt = Foam::rootVGreat;

  // compute OpenFOAM's desired time step
  _desired_dt = solver().computeDeltaT();
  // reset MOOSE's time step and return
  _dt = dt_tmp;
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
