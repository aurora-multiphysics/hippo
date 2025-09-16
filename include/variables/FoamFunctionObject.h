#pragma once

#include "FoamVariableField.h"
#include "functionObject.H"

#include <volFields.H>

// Class for mirroring functionObjects
class FoamFunctionObject : public FoamVariableField
{
public:
  explicit FoamFunctionObject(const InputParameters & params);

  // function that executes functionObject and transfers variable from
  // OpenFOAM variable to MOOSE
  virtual void transferVariable();

  // Destroys functionObject pointer
  ~FoamFunctionObject() { delete _shadow_fo; };

private:
  // function to construct the functionObject
  Foam::functionObject * _getFunctionObject(Foam::dictionary fo_dict);

  // The function object pointer
  Foam::functionObject * _shadow_fo;
};
