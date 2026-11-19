#pragma once

#include "FoamVariable.h"
#include "MooseTypes.h"
#include "WallQuantitiesBase.h"

/**
 * Base class for transferring derived OpenFOAM wall quantities to MOOSE variables.
 * Delegates quantity evaluation to the selected WallQuantities implementation
 * and stores the results in registered boundary buffers before optional MOOSE mirroring.
 */
class FoamWallVariableBase : public FoamScalarVariable
{
public:
  static InputParameters validParams();

  FoamWallVariableBase(const InputParameters & params);

  void syncVariable() override;

protected:
  std::unique_ptr<WallQuantitiesBase> _wall_quantities;
  virtual const Foam::scalarField getFoamField(const SubdomainName & boundary) = 0;
};
