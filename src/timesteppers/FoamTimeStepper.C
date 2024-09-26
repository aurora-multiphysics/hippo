#include "FoamTimeStepper.h"
#include "FoamProblem.h"
#include "FoamInterface.h"

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
  _interface->setDT(_dt);
  return dt;
}

// TODO: Need to work out how to
// negotiate DT in picard iteration
Real
FoamTimeStepper::computeDT()
{
  _interface->setDT(_dt);
  return _dt;
}

void
FoamTimeStepper::init()
{
  TimeStepper::init();
  _interface->setCurrentTime(_time);
  _interface->setEndT(_end_time);
  _interface->setDT(_dt);
}

// Local Variables:
// mode: c++
// End:
