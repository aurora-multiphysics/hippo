#include "FoamTestBC.h"

registerMooseObject("hippoTestApp", FoamTestBC);

InputParameters
FoamTestBC::validParams()
{
  auto params = FoamVariableBCBase::validParams();
  params.set<std::string>("_foam_bc_type") = "fixedValue";
  return params;
}
