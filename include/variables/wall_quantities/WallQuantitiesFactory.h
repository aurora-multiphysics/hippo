#pragma once

#include "MooseEnum.h"
#include "MooseObject.h"
#include "WallQuantitiesBase.h"
#include <memory>

namespace Hippo
{
namespace internal
{
// Returns MooseEnum with solver options for wall quantities
inline MooseEnum
getWallQuantitiesEnum()
{
  return MooseEnum("FLUID MULTIPHASE_EULER", "FLUID");
}

// Factory function for wall quantities
std::unique_ptr<WallQuantitiesBase> createWallQuantities(const MooseObject & moose_object,
                                                         const MooseEnum & wall_quantity);
}
}
