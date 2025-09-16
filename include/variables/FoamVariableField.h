#pragma once

#include "MooseTypes.h"
#include "FoamMesh.h"
#include "MooseVariableConstMonomial.h"

class FoamVariableField : public MooseVariableConstMonomial
{
public:
  static InputParameters validParams();

  explicit FoamVariableField(const InputParameters & params);

  // transfer variable from OpenFOAM field to MOOSE variable
  virtual void transferVariable();

protected:
  // variable name or functionObject to be shadowed
  std::string _foam_variable;

  // Pointer to the FoamMesh object
  FoamMesh * _mesh;
};
