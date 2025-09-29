#pragma once

#include "Moose.h"
#include "MooseTypes.h"
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
  MooseVariableFieldBase & _moose_var;

  // Create Auxilliary variable
  MooseVariableFieldBase & createMooseVariable(std::string name, const InputParameters & params);

  // Pointer to the FoamMesh object
  FoamMesh * _mesh;
};
