#include "FoamProblem.h"
#include "FoamControlledTimeStepper.h"
#include "MooseError.h"
#include "scalar.H"

#include <TimeStepper.h>
#include <Transient.h>

registerMooseObject("hippoApp", FoamControlledTimeStepper);

InputParameters
FoamControlledTimeStepper::validParams()
{
  auto params = TimeStepper::validParams();
  return params;
}

FoamControlledTimeStepper::FoamControlledTimeStepper(InputParameters const & params) : TimeStepper(params)
{
  auto problem = dynamic_cast<FoamProblem *>(&_app.feProblem());
  if (!problem)
  {
    mooseError("FoamTimeStepper expects to be used with FoamProblem");
  }
}

Real
FoamControlledTimeStepper::computeDT(){
  Foam::solver & foam_solver{solver().solver()};

  if (!_dt_adjustable)
    return foam_solver.runTime.deltaTValue();

  Real deltaT = std::min(foam_solver.maxDeltaT(),
                         foam_solver.runTime.functionObjects().maxDeltaT());

  if (deltaT < Foam::rootVGreat){
    deltaT = std::min(Foam::solver::deltaTFactor * foam_solver.runTime.deltaTValue(), deltaT);
    std::cout << "FoamControlledTimeStepper: " << deltaT <<"\n";
    return deltaT;
  }
  mooseError("deltaT nust be less that rootVGreat");
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

void FoamControlledTimeStepper::init()
{
  TimeStepper::init();

  Foam::solver & foam_solver{solver().solver()};
  _dt_adjustable = foam_solver.runTime.controlDict().lookupOrDefault("adjustTimeStep", false);
}