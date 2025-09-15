#pragma once

#include "FoamVariableBase.h"
#include "functionObject.H"

#include <volFields.H>

class FoamFunctionObject : public FoamVariableBase
{
public:
  explicit FoamFunctionObject(const InputParameters & params);

  virtual void transferVariable();
  ~FoamFunctionObject();

private:
  Foam::functionObject * _getFunctionObject(Foam::dictionary fo_dict);
  Foam::volScalarField const * _field_shadow = nullptr;
  Foam::functionObject * _shadow_fo;
};
