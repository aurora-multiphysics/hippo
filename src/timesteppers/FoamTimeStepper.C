#include "FoamTimeStepper.h"
#include "FoamProblem.h"
#include "FoamInterface.h"

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
    mooseError("FoamTimeStepper expects to be used with foamProblem");
  }
  _interface = problem->shareInterface();
}

Real
FoamTimeStepper::computeInitialDT()
{
  return _interface->getDT();
}

// TODO: Need to work out how to
// negotiate DT in picard iteration
Real
FoamTimeStepper::computeDT()
{
  return _interface->getDT();
}

void
FoamTimeStepper::init()
{
  TimeStepper::init();
  _time = _interface->getBeginT();
  _end_time = _interface->getEndT();
}

// Local Variables:
// mode: c++
// End:
