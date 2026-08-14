#include "FoamTestBC.h"

registerMooseObject("hippoTestApp", FoamTestBC);

InputParameters
FoamTestBC::validParams()
{
  auto params = FoamVariableBCBase::validParams();
  return params;
}
