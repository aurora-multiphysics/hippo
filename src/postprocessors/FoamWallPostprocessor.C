#include "FoamSidePostprocessor.h"
#include "FoamWallPostprocessor.h"
#include "InputParameters.h"
#include "MooseEnum.h"
#include "MooseTypes.h"
#include "WallQuantitiesFactory.h"
#include "libmesh/libmesh_common.h"
#include <scalarField.H>

InputParameters
FoamWallPostprocessor::validParams()
{
  InputParameters params = FoamSidePostprocessor::validParams();
  params.addParam<MooseEnum>("wall_quantity",
                             Hippo::internal::getWallQuantitiesEnum(),
                             "Wall quantity based on fluid solver type");
  return params;
}

FoamWallPostprocessor::FoamWallPostprocessor(const InputParameters & params)
  : FoamSidePostprocessor(params),
    _wall_quantities(
        Hippo::internal::createWallQuantities(*this, getParam<MooseEnum>("wall_quantity"))),
    _value(0.)
{
}

PostprocessorValue
FoamWallPostprocessor::getValue() const
{
  return _value;
}

Real
FoamWallPostprocessor::integrateField()
{
  Real value = 0.;
  // loop over boundary ids
  for (auto & boundary : _boundary)
  {
    auto & areas = getFvMesh().boundary()[boundary].magSf();
    Foam::scalarField && var_array = wallField(boundary);
    // Integrate
    for (int i = 0; i < var_array.size(); ++i)
    {
      value += var_array[i] * areas[i];
    }
  }
  // sum over ranks
  gatherSum(value);

  return value;
}
