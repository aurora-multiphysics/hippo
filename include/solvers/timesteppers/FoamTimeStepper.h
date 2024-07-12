#pragma once

#include "TimeStepper.h"
#include "FoamProblem.h"
#include "FoamInterface.h"

/* Simple class that calls the interface to get/set timestep parameters */

class FoamTimeStepper : public TimeStepper
{
public:
  FoamTimeStepper(InputParameters const & params);
  static InputParameters validParams();

  virtual Real computeInitialDT();
  virtual Real computeDT();
  virtual void init();

protected:
  Hippo::FoamInterface * _interface;
};

// Local Variables:
// mode: c++
// End:
