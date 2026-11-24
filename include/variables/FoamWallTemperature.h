#pragma once

#include "FoamFieldBase.h"
#include "FoamWallVariableBase.h"
#include "InputParameters.h"
#include "WallQuantitiesBase.h"
#include <memory>
#include <scalarField.H>

class FoamWallTemperature : public FoamWallVariableBase
{
public:
  static InputParameters validParams();

  FoamWallTemperature(const InputParameters & params);

protected:
  virtual const Foam::scalarField getFoamField(const SubdomainName & boundary) override;
};
