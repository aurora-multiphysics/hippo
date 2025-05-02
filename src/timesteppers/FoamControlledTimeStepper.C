#include "FoamProblem.h"
#include "FoamControlledTimeStepper.h"

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
  Foam::solver const & foam_solver{solver().solver()};

  if (!_dt_adjustable)
    return _foam_initial_dt;

  // This code has been adapted from OpenFOAM's adjustDeltaT to determine the time-step that
  // OpenFOAM will use on the next time step so MOOSE can predict it.
  Real deltaT =
      std::min(foam_solver.maxDeltaT(), foam_solver.runTime.functionObjects().maxDeltaT());

  if (deltaT >= Foam::rootVGreat)
    mooseError("Computed OpenFOAM time step must be less that rootVGreat: ",
               deltaT,
               " >= ",
               Foam::rootVGreat,
               ".");

  return std::min(Foam::solver::deltaTFactor * foam_solver.runTime.deltaTValue(), deltaT);
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

  Foam::solver const & foam_solver{solver().solver()};

  // determine if OpenFOAM's time-step is adjustable in controlDict
  _dt_adjustable = foam_solver.runTime.controlDict().lookupOrDefault("adjustTimeStep", false);

  if (!_dt_adjustable)
    _foam_initial_dt = foam_solver.runTime.deltaTValue();
}
