#include "FoamWallVariableBase.h"
#include "WallQuantitiesFactory.h"

InputParameters
FoamWallVariableBase::validParams()
{
  auto params = FoamScalarVariable::validParams();
  params.addParam<MooseEnum>("wall_quantity",
                             Hippo::internal::getWallQuantitiesEnum(),
                             "Wall quantity based on fluid solver type");
  params.addRequiredParam<std::vector<BoundaryName>>("boundary", "Boundaries to evaluate.");
  params.set<bool>("create_aux_variable") = true;
  params.suppressParameter<std::string>("foam_variable");
  params.suppressParameter<bool>("volume");
  return params;
}

FoamWallVariableBase::FoamWallVariableBase(const InputParameters & params)
  : FoamScalarVariable(params),
    _wall_quantities(
        Hippo::internal::createWallQuantities(*this, getParam<MooseEnum>("wall_quantity")))
{
}

void
FoamWallVariableBase::syncVariable()
{
  auto & accessor = getFieldAccessor();
  for (const auto & boundary : accessor.boundaryNames())
    accessor.boundaryField(boundary) = getFoamField(boundary);

  FoamScalarVariable::syncVariable();
}
