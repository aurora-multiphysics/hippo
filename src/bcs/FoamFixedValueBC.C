#include "FoamFixedValueBC.h"

registerMooseObject("hippoApp", FoamFixedValueBC);

InputParameters
FoamFixedValueBC::validParams()
{
  auto params = FoamVariableBCBase::validParams();

  params.addClassDescription("A FoamBC that imposes a fixed value dirichlet boundary condition "
                             "on the OpenFOAM simulation");
  return params;
}

FoamFixedValueBC::FoamFixedValueBC(const InputParameters & parameters)
  : FoamVariableBCBase(parameters, FoamBCType::fixedValue)
{
}
