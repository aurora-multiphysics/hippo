#include "FoamFieldBase.h"
#include "FoamWallVariableBase.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "WallQuantitiesFactory.h"
#include "FoamMesh.h"
#include "hippoUtils.h"
#include "FoamProblem.h"

InputParameters
FoamWallVariableBase::validParams()
{
  auto params = FoamFieldBase::validParams();
  params.addParam<MooseEnum>("wall_quantity",
                             Hippo::internal::getWallQuantitiesEnum(),
                             "Wall quantity based on fluid solver type");
  params.addRequiredParam<std::vector<SubdomainName>>("boundary", "Boundary this object mirrors.");
  return params;
}

FoamWallVariableBase::FoamWallVariableBase(const InputParameters & params)
  : FoamFieldBase(params),
    _wall_quantities(
        Hippo::internal::createWallQuantities(*this, getParam<MooseEnum>("wall_quantity")))
{
}

void
FoamWallVariableBase::transferVariable()
{
  THREAD_ID tid = getParam<THREAD_ID>("_tid");
  auto & moose_var = getFoamProblem().getVariable(tid, _name);

  const auto & subdomains{getParam<std::vector<SubdomainName>>("boundary")};

  for (const auto & subdomain : subdomains)
  {
    Hippo::internal::copyFieldFoamToMoose(
        getFoamMesh(), getFoamField(subdomain), moose_var, getFoamMesh().getSubdomainID(subdomain));
  }

  moose_var.sys().solution().close();
}
