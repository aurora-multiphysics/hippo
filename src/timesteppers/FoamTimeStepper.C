#include "FoamProblem.h"
#include "FoamTimeStepper.h"

#include <TimeStepper.h>
#include <Transient.h>

registerMooseObject("hippoApp", FoamTimeStepper);

InputParameters
FoamTimeStepper::validParams()
{
  auto params = TimeStepper::validParams();
  return params;
}

FoamTimeStepper::FoamTimeStepper(InputParameters const & params) : TimeStepper(params)
{
  auto problem = dynamic_cast<FoamProblem *>(&_app.feProblem());
  if (!problem)
  {
    mooseError("FoamTimeStepper expects to be used with FoamProblem");
  }
}

Real
FoamTimeStepper::computeInitialDT()
{
  auto dt = _executioner.parameters().get<double>("dt");
  solver().setTimeDelta(_dt);
  return dt;
}

Real
FoamTimeStepper::computeDT()
{
  solver().setTimeDelta(_dt);
  return _dt;
}

void
FoamTimeStepper::init()
{
  TimeStepper::init();
  solver().setCurrentTime(_time);
  solver().setEndTime(_end_time);
  solver().setTimeDelta(_dt);
}
