#pragma once

#include "FoamWallVariableBase.h"
#include "InputParameters.h"

class FoamWallHeatFlux : public FoamWallVariableBase
{
public:
  static InputParameters validParams();

  FoamWallHeatFlux(const InputParameters & params);

protected:
  virtual const Foam::scalarField getFoamField(const SubdomainName & boundary) override;
};
