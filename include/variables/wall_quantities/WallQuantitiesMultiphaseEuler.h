#pragma once

#include "MooseObject.h"
#include "WallQuantitiesBase.h"
#include <functional>
#include <fvPatchFieldsFwd.H>
#include <scalarField.H>
#include <phaseSystem.H>

class WallQuantitiesMultiphaseEuler : public WallQuantitiesBase
{
public:
  explicit WallQuantitiesMultiphaseEuler(const MooseObject * moose_object);

  virtual Foam::scalarField wallTemperature(const SubdomainName & boundary) override;
  virtual Foam::scalarField wallHeatFlux(const SubdomainName & boundary) override;
  virtual Foam::scalarField internalTemperature(const Foam::labelUList & cells) override;

protected:
  std::optional<std::reference_wrapper<const Foam::phaseSystem>> _phase_system;
};
