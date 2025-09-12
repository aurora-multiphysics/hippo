#pragma once

#include "MooseVariableConstMonomial.h"

#include <volFields.H>

class FoamVariableField : public MooseVariableConstMonomial
{
public:
  static InputParameters validParams();

  explicit FoamVariableField(const InputParameters & params);

  void transferVariable();

private:
  Foam::volScalarField const * _field_shadow = nullptr;
  std::string _foam_variable;
};
