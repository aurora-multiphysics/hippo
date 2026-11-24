#pragma once

#include "BulkTemperatureUserObject.h"
#include "HippoInterface.h"
#include "MooseObject.h"
#include "MooseTypes.h"
#include <UList.H>
#include <fvPatch.H>
#include <scalarField.H>

class UserObject;

/*
 * Base class for handling wall-related solver-specific thermal quantities including
 * wall temperature, wall heat flux, fluid temperature and HTC
 */
class WallQuantitiesBase : protected HippoInterface
{
public:
  explicit WallQuantitiesBase(const MooseObject * moose_object);
  // Virtual destructor
  virtual ~WallQuantitiesBase() = default;
  // Wall temperature on patch boundary.
  virtual Foam::scalarField wallTemperature(const SubdomainName & boundary) = 0;
  // Wall heat flux on patch boundary.
  virtual Foam::scalarField wallHeatFlux(const SubdomainName & boundary) = 0;
  // internal temperature on selected internal cells.
  virtual Foam::scalarField internalTemperature(const Foam::labelUList & cells) = 0;
  // HTC computed from T_w, q_w and bulk temperature user object
  virtual Foam::scalarField heatTransferCoefficient(const SubdomainName & boundary,
                                                    BulkTemperatureUserObject & t_bulk_uo);

protected:
  const Foam::fvPatch & getFoamPatch(const SubdomainName & boundary);
};
