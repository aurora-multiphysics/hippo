#include "FoamHeatTransferCoeff.h"
#include "FoamProblem.h"
#include "FoamWallVariableBase.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "Registry.h"
#include "UserObject.h"
#include <scalar.H>
#include <scalarField.H>
#include <string>
#include <volFieldsFwd.H>

registerMooseObject("hippoApp", FoamHeatTransferCoeff);

InputParameters
FoamHeatTransferCoeff::validParams()
{
  InputParameters params = FoamWallVariableBase::validParams();
  params.addRequiredParam<std::vector<UserObjectName>>(
      "bulk_temperature_uo", "UserObject describing the bulk temperature");
  params.addClassDescription("Computes and transfers the wall heat transfer coefficient as "
                             "h = q / (T_wall - T_bulk) on the selected OpenFOAM boundaries. "
                             "Bulk temperature is supplied by a user object for each boundary.");

  return params;
}

FoamHeatTransferCoeff::FoamHeatTransferCoeff(const InputParameters & params)
  : FoamWallVariableBase(params), _t_bulk_uo_name()
{
  const auto & boundaries = getParam<std::vector<SubdomainName>>("boundary");
  const auto & t_bulks = getParam<std::vector<UserObjectName>>("bulk_temperature_uo");

  if (t_bulks.size() != boundaries.size())
    mooseError("Sizes of user object and boundary list should be the same.");

  for (auto i = 0lu; i < boundaries.size(); ++i)
  {
    _t_bulk_uo_name[boundaries[i]] = t_bulks[i];
  }
}

const Foam::scalarField
FoamHeatTransferCoeff::getFoamField(const SubdomainName & boundary)
{
  return _wall_quantities->heatTransferCoefficient(
      boundary, getFoamProblem().getUserObject<UserObject>(_t_bulk_uo_name.at(boundary)));
}
