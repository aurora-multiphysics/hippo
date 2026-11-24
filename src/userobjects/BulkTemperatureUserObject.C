#include "BulkTemperatureUserObject.h"
#include "InputParameters.h"

InputParameters
BulkTemperatureUserObject::validParams()
{
  return GeneralUserObject::validParams();
}

BulkTemperatureUserObject::BulkTemperatureUserObject(const InputParameters & params)
  : GeneralUserObject(params)
{
}
