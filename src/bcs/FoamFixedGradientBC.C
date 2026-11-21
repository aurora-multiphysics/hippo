#include "FoamFixedGradientBC.h"

registerMooseObject("hippoApp", FoamFixedGradientBC);

InputParameters
FoamFixedGradientBC::validParams()
{
  auto params = FoamVariableBCBase::validParams();

  params.addClassDescription("A FoamBC that imposes a fixed gradient boundary condition "
                             "on the OpenFOAM simulation");
  return params;
}

FoamFixedGradientBC::FoamFixedGradientBC(const InputParameters & parameters)
  : FoamVariableBCBase(parameters, FoamBCType::fixedGradient)
{
}
