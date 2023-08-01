#if 0
#pragma once

#include "FoamProblemBase.h"
#include "icoFoamInterface.h"
class icoFoamProblem : public FoamProblemBase
{
public:
  static InputParameters validParams();
  icoFoamProblem(InputParameters const &params);
  virtual void externalSolve();
  virtual void syncSolutions(Direction dir) {}
  virtual bool converged() {return true;}
  protected:
  Hippo::icoFoam::FoamApp app_;
};
#endif
// Local Variables:
// mode: c++
// End:
