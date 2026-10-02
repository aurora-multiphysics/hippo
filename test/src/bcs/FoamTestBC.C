#include "FoamTestBC.h"

registerMooseObject("hippoTestApp", FoamTestBC);

InputParameters
FoamTestBC::validParams()
{
  auto params = FoamVariableBCBase::validParams();
  return params;
}

void
FoamTestBC::constructFoamPatch(Foam::label patch_id)
{
  constructFixedValuePatch(patch_id);
}
