#include "HippoInterface.h"
#include "MooseObject.h"
#include "UserObject.h"
#include "WallQuantitiesBase.h"
#include <fvPatch.H>
#include <scalarField.H>

WallQuantitiesBase::WallQuantitiesBase(const MooseObject * moose_object)
  : HippoInterface(moose_object)
{
}

Foam::scalarField
WallQuantitiesBase::heatTransferCoefficient(const SubdomainName & boundary, UserObject & t_bulk_uo)
{
  const Foam::scalarField T_wall{wallTemperature(boundary)};
  const Foam::scalarField q{wallHeatFlux(boundary)};
  const Foam::fvPatch & patch = getFoamPatch(boundary);

  const Foam::vectorField & cellCenters{patch.Cf()};
  const Foam::scalar eps = Foam::ROOTVSMALL;

  t_bulk_uo.execute();
  Foam::scalarField htc{patch.size(), 0};
  for (int i = 0; i < htc.size(); ++i)
  {
    const Point p{cellCenters[i].x(), cellCenters[i].y(), cellCenters[i].z()};
    const Foam::scalar T_ref = t_bulk_uo.spatialValue(p);
    htc[i] = q[i] / (T_wall[i] - T_ref + eps);
  }

  return htc;
}

const Foam::fvPatch &
WallQuantitiesBase::getFoamPatch(const SubdomainName & boundary)
{
  const auto patch_id = getFvMesh().boundary().findIndex(boundary);
  if (patch_id < 0)
    mooseError("Boundary '", boundary, "' not found in OpenFOAM mesh.");
  return getFvMesh().boundary()[patch_id];
}
