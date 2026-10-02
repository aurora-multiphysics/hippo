#include "FoamWallAverageHTC.h"
#include "FoamProblem.h"
#include "FoamWallPostprocessor.h"
#include "InputParameters.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include "UserObject.h"
#include <scalarField.H>
#include <string>

registerMooseObject("hippoApp", FoamWallAverageHTC);

InputParameters
FoamWallAverageHTC::validParams()
{
  InputParameters params = FoamWallPostprocessor::validParams();
  params.addClassDescription(
      "Computes the area-weighted average of local wall heat transfer coefficients in W/(m^2 K) "
      "over the selected OpenFOAM boundary patches, using the supplied bulk temperature user "
      "objects and heat flux positive into the fluid.");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "bulk_temperature_uo",
      "One bulk temperature user object per boundary, in the same order as boundary.");
  return params;
}

FoamWallAverageHTC::FoamWallAverageHTC(const InputParameters & params)
  : FoamWallPostprocessor(params), _t_bulk_uo_names()
{
  const auto & t_bulks = getParam<std::vector<UserObjectName>>("bulk_temperature_uo");
  if (t_bulks.size() != _boundary.size())
    mooseError("Exactly one bulk temperature user object must be provided per boundary.");

  for (auto i = 0lu; i < _boundary.size(); ++i)
  {
    _t_bulk_uo_names[_boundary[i]] = t_bulks[i];
  }
}

Foam::scalarField
FoamWallAverageHTC::wallField(const std::string & boundary)
{
  return _wall_quantities->heatTransferCoefficient(
      boundary, getFoamProblem().getUserObject<UserObject>(_t_bulk_uo_names.at(boundary)));
}

void
FoamWallAverageHTC::compute()
{
  _value = averageField();
}
