#pragma once

#include "FoamVariableBase.h"

#include <volFields.H>

class FoamVariableField : public FoamVariableBase
{
public:
  explicit FoamVariableField(const InputParameters & params);

private:
  Foam::volScalarField const * _field_shadow = nullptr;
};
