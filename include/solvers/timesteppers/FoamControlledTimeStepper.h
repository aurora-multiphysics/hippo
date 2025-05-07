#pragma once

#include "FoamProblem.h"
#include "FoamSolver.h"
#include "functionObject.H"

#include <InputParameters.h>
#include <TimeStepper.h>

/*
Time stepper that allows OpenFOAM to control the time step enabling features such as CFL
 daptive time steps. The intention is to allows the current time step in OpenFOAM
 to be exposed to MOOSE
 */

namespace Foam
{
namespace functionObjects
{
// Function object to tell OpenFOAM what MOOSE's dt is
class mooseDeltaT : public functionObject
{
private:
  const Real & dt_;

public:
  TypeName("mooseDeltaT");
  mooseDeltaT(const word & name, const Time & runTime, const Real & dt)
    : functionObject(name, runTime), dt_(dt)
  {
  }

  wordList fields() const { return wordList::null(); }

  bool executeAtStart() const { return false; }

  bool execute() { return true; }
  bool write() { return true; }
  scalar maxDeltaT() const { return dt_; }
};
}
}

class FoamControlledTimeStepper : public TimeStepper
{
public:
  FoamControlledTimeStepper(InputParameters const & params);
  static InputParameters validParams();

  // Get initial time step from OpenFOAM input file
  virtual Real computeInitialDT() { return computeDT(); };

  /* Read time step from OpenFOAM
    - Make sure the time step duration is computed in the current step
  */
  virtual Real computeDT();

  // Consider how to communicate starting time for this stepper
  // e.g. after a restart this would need to be executed after the
  // OpenFOAM restart.
  virtual void init();

private:
  // These two variables are needed depending on how the time-stepper is initialised
  Hippo::FoamSolver & solver() { return problem()->solver(); }
  FoamProblem * problem();
  // Variables to determine whether an adjustable time step is used in OF and
  // what it is.
  bool _dt_adjustable = false;
  Real _foam_initial_dt = 0.;
};
