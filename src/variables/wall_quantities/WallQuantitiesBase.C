#include "WallQuantitiesBase.h"
#include <fvPatch.H>
#include <scalar.H>
#include <scalarField.H>
#include <cmath>

WallQuantitiesBase::WallQuantitiesBase(const MooseObject * moose_object)
  : HippoInterface(moose_object)
{
}

Foam::scalarField
WallQuantitiesBase::heatTransferCoefficient(const SubdomainName & boundary,
                                            BulkTemperatureUserObject & t_bulk_uo)
{
  const Foam::scalarField T_wall{wallTemperature(boundary)};
  const Foam::scalarField q{wallHeatFlux(boundary)};
  const Foam::fvPatch & patch = getFoamPatch(boundary);

  const Foam::vectorField & cellCenters{patch.Cf()};

  t_bulk_uo.execute();
  Foam::scalarField htc{patch.size(), 0};
  for (int i = 0; i < htc.size(); ++i)
  {
    const Point p{cellCenters[i].x(), cellCenters[i].y(), cellCenters[i].z()};
    const Foam::scalar T_ref = t_bulk_uo.spatialValue(p);

    Foam::scalar deltaT = T_wall[i] - T_ref;
    deltaT = (std::abs(deltaT) > 1e-10) ? deltaT : std::copysign(1e-10, deltaT);
    htc[i] = q[i] / deltaT;
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
