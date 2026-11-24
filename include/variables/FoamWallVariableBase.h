#pragma once

#include "FoamFieldBase.h"
#include "MooseTypes.h"
#include "WallQuantitiesBase.h"

/**
 * Base class for transferring derived OpenFOAM wall quantities to MOOSE variables.
 * Delegates quantity evaluation to the selected WallQuantities implementation
 * and maps boundary-face values onto the corresponding MOOSE elements.
 */
class FoamWallVariableBase : public FoamFieldBase
{
public:
  static InputParameters validParams();

  FoamWallVariableBase(const InputParameters & params);

  void transferVariable() override;

  std::string foamVariable() const override { return ""; };

protected:
  std::unique_ptr<WallQuantitiesBase> _wall_quantities;
  virtual const Foam::scalarField getFoamField(const SubdomainName & boundary) = 0;
};
