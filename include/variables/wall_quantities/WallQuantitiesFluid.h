#pragma once

#include "InputParameters.h"
#include "MooseObject.h"
#include "MooseTypes.h"
#include "WallQuantitiesBase.h"
#include <fvPatchFieldsFwd.H>

class WallQuantitiesFluid : public WallQuantitiesBase
{
public:
  explicit WallQuantitiesFluid(const MooseObject * moose_object);

  virtual Foam::scalarField wallTemperature(const SubdomainName & boundary) override;
  virtual Foam::scalarField wallHeatFlux(const SubdomainName & boundary) override;
  virtual Foam::scalarField internalTemperature(const Foam::labelUList & cells) override;

protected:
  const std::string & getTFieldName();
};
