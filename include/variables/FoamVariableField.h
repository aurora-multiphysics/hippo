#pragma once

#include "MooseObject.h"
#include "FoamMesh.h"

class FoamVariableField : public MooseObject
{
public:
  static InputParameters validParams();

  explicit FoamVariableField(const InputParameters & params);

  // transfer variable from OpenFOAM field to MOOSE variable
  virtual void transferVariable();

  // returns the name of the foam variable this object shadows
  std::string foamVariable() const { return _foam_variable; };

protected:
  // variable name or functionObject to be shadowed
  std::string _foam_variable;

  // moose variable that shadows the OpenFOAM variable
  VariableName _moose_var_name;

  // Pointer to the FoamMesh object
  FoamMesh * _mesh;
};
