#include "FoamProblem.h"
#include "FoamTimeStepper.h"

#include <TimeStepper.h>
#include <Transient.h>

#include <cmath>

registerMooseObject("hippoApp", FoamTimeStepper);

namespace
{
bool
isClose(double a, double b, double tolerance = 1e-12)
{
  return std::fabs(a - b) <= tolerance;
}
}

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
  // The problem here is that floating point errors were sometimes causing
  // MOOSE to ask for tiny time steps from OpenFOAM when sub-cycling. This
  // would result in an infinite loop where the MOOSE would never consider the
  // applications synchronised.
  // Setting `_executioner.setTimestepTolerance` did not seem to help.
  // This is potentially not ideal, but if we look for small changes to the
  // time step size compared to the previous step, then we can prevent that
  // small change by reverting to the previous time step size. We use the
  // OpenFOAM solver as a proxy for the previous time step size (as we set that
  // time step size every time we call this function).
  // This solution allows MOOSE to set large time step deviations (to do a real
  // synchronization), but should prevent small numerical errors accumulating.
  if (auto foam_dt = solver().timeDelta(); isClose(_dt, foam_dt))
  {
    solver().setTimeDelta(foam_dt);
    return foam_dt;
  }
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
