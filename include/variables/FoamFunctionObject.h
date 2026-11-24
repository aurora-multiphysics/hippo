#pragma once

#include "FoamVariable.h"
#include "functionObject.H"

#include <volFields.H>

// Class for mirroring functionObjects
class FoamFunctionObject : public FoamScalarVariable
{
public:
  static InputParameters validParams();
  explicit FoamFunctionObject(const InputParameters & params);

  /// Execute the function object and optionally mirror its field into MOOSE.
  void syncVariable() override;

  // Destroys functionObject pointer
  ~FoamFunctionObject() { delete _shadow_fo; };

private:
  // function to construct the functionObject
  Foam::functionObject * _getFunctionObject(Foam::dictionary fo_dict);

  // The function object pointer
  Foam::functionObject * _shadow_fo;
};
