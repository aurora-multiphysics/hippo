#include "WallQuantitiesFactory.h"
#include "WallQuantitiesFluid.h"
#include "WallQuantitiesMultiphaseEuler.h"

std::unique_ptr<WallQuantitiesBase>
Hippo::internal::createWallQuantities(const MooseObject & moose_object,
                                      const MooseEnum & wall_quantity)
{
  if (wall_quantity == "FLUID")
  {
    return std::make_unique<WallQuantitiesFluid>(&moose_object);
  }
  else if (wall_quantity == "MULTIPHASE_EULER")
  {
    return std::make_unique<WallQuantitiesMultiphaseEuler>(&moose_object);
  }

  mooseError("WallQuantity '", wall_quantity, "' not found.");
}
