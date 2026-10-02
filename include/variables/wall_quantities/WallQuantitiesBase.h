#pragma once

#include "HippoInterface.h"
#include "InputParameters.h"
#include "MooseObject.h"
#include "MooseTypes.h"
#include <UList.H>
#include <fvPatch.H>
#include <fvPatchFieldsFwd.H>
#include <scalarField.H>
#include "UserObject.h"

/*
 * Base class for handling wall-related solver-specific thermal quantities including
 * wall temperature, wall heat flux, fluid temperature and HTC
 */
class WallQuantitiesBase : protected HippoInterface
{
public:
  explicit WallQuantitiesBase(const MooseObject * moose_object);
  // Wall temperature on patch boundary.
  virtual Foam::scalarField wallTemperature(const SubdomainName & boundary) = 0;
  // Wall heat flux on patch boundary.
  virtual Foam::scalarField wallHeatFlux(const SubdomainName & boundary) = 0;
  // internal temperature on selected internal cells.
  virtual Foam::scalarField internalTemperature(const Foam::labelUList & cells) = 0;
  // HTC computed from T_w, q_w and bulk temperature user object
  virtual Foam::scalarField heatTransferCoefficient(const SubdomainName & boundary,
                                                    UserObject & t_bulk_uo);

protected:
  const Foam::fvPatch & getFoamPatch(const SubdomainName & boundary);
};
