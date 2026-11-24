#pragma once

#include "FoamVariableBCBase.h"
#include "InputParameters.h"

class FoamFixedGradientBC : public FoamVariableBCBase
{
public:
  // Validate input file parameters
  static InputParameters validParams();

  // Constructor
  explicit FoamFixedGradientBC(const InputParameters & parameters);

  // Impose boundary conditions (to be called from FoamProblem class)
  virtual void imposeBoundaryCondition(bool initialisation = false) override;

protected:
  void constructFoamPatch(Foam::label patch_id) override { constructFixedGradientPatch(patch_id); }
};
