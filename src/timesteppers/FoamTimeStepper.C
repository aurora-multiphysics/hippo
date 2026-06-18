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
  : TimeStepper(params), _foam_dt{}, _desired_dt{}
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

  if (_t_step == 0)
    return solver().getTimeDelta();
  // Not ideal, but for MOOSE to get an accurate deltaT
  // preSolve must be called as this updates the BCs.
  solver().preSolve();

  auto & moose_dt = solver().getDeltaTFunctionObject();
  // Tells the mooseDelta function object what the previous desired time
  // step was so it can work out whether there was a MOOSE induced cutback.
  moose_dt.setOldDesiredDt(_desired_dt);

  // Ensure MOOSE gets OpenFOAM's time step unaffected by the mooseDeltaT
  // functionObject.
  moose_dt.disable();

  // compute OpenFOAM's desired time step
  _desired_dt = solver().computeDeltaT();

  // reenable the function object
  moose_dt.enable();

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
    solver().setDeltaTAdjustable(false);
    return;
  }
  // determine if OpenFOAM's time-step is adjustable in controlDict
  _dt_adjustable = solver().isDeltaTAdjustable();
  _desired_dt = solver().getTimeDelta();
  if (!_dt_adjustable)
    _foam_dt = solver().getTimeDelta();
}
