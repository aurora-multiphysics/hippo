#include "FoamSideIntegratedValue.h"
#include "InputParameters.h"
#include "MooseTypes.h"

registerMooseObject("hippoApp", FoamSideIntegratedValue);

InputParameters
FoamSideIntegratedValue::validParams()
{
  auto params = FoamSideIntegratedBase::validParams();
  params.addRequiredParam<std::string>("foam_variable",
                                       "Foam variable to be averaged over a boundary patch.");
  return params;
}

FoamSideIntegratedValue::FoamSideIntegratedValue(const InputParameters & params)
  : FoamSideIntegratedBase(params), _foam_variable(getParam<std::string>("foam_variable"))
{
  // determine if this is a vector scalar, ahead of computation
  if (!_foam_mesh->foundObject<Foam::volVectorField>(_foam_variable) &&
      !_foam_mesh->foundObject<Foam::volScalarField>(_foam_variable))
    mooseError("No Foam scalar or vector called '", _foam_variable, "'.");
}

void
FoamSideIntegratedValue::compute()
{
  _value = integrateValue(_foam_variable);
}
