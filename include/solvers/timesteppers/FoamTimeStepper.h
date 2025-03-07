#pragma once

#include "FoamProblem.h"
#include "FoamSolver.h"

#include <TimeStepper.h>

/**
 * TimeStepper implementation for OpenFOAM.
 *
 * This is a simple implementation that takes the current time, final time and
 * time step size from MOOSE and sets them on the OpenFOAM solver.
 *
 * This does not rewrite the OpenFOAM case's controlDict, but any time-step
 * related settings in the controlDict file will be ignored.
 */
class FoamTimeStepper : public TimeStepper
{
public:
  FoamTimeStepper(InputParameters const & params);
  static InputParameters validParams();

  /// Initial time-step size comes from the MOOSE input file.
  virtual Real computeInitialDT();
  /**
   * Read the time-step MOOSE wants to take and set it on the OpenFOAM problem.
   *
   * The main idea behind this is to enable MOOSE's fixed point iterations,
   * where MOOSE may want to change the current time/time-step and the OpenFOAM
   * solve needs to be updated to reflect this.
   */
  virtual Real computeDT();
  /// Set the initial time, final time and time step size on the OpenFOAM
  /// problem.
  virtual void init();

private:
  Hippo::FoamSolver & solver() { return problem()->solver(); }
  FoamProblem * problem();
};
