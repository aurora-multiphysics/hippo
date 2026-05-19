#include "FixedBulkTemperature.h"
#include "GeneralUserObject.h"
#include "InputParameters.h"

registerMooseObject("hippoApp", FixedBulkTemperature);

InputParameters
FixedBulkTemperature::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<Real>("T_bulk", "Fixed reference temperature");

  return params;
}

FixedBulkTemperature::FixedBulkTemperature(const InputParameters & params)
  : GeneralUserObject(params), _T_bulk(getParam<Real>("T_bulk"))
{
}

Real
FixedBulkTemperature::spatialValue([[maybe_unused]] const Point & point) const
{
  return _T_bulk;
}
