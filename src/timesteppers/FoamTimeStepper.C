#include "FoamTimeStepper.h"
#include "FoamProblem.h"

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
  _interface = problem->shareInterface();
}

Real
FoamTimeStepper::computeInitialDT()
{
  auto dt = _executioner.parameters().get<double>("dt");
  _interface->setTimeDelta(_dt);
  return dt;
}

Real
FoamTimeStepper::computeDT()
{
  _interface->setTimeDelta(_dt);
  return _dt;
}

void
FoamTimeStepper::init()
{
  TimeStepper::init();
  _interface->setCurrentTime(_time);
  _interface->setEndTime(_end_time);
  _interface->setTimeDelta(_dt);
}
