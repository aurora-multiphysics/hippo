#pragma once

#include "FoamFieldBase.h"
#include "FoamMesh.h"

class FoamVariableField : public FoamFieldBase
{
public:
  static InputParameters validParams();

  explicit FoamVariableField(const InputParameters & params);

  // transfer variable from OpenFOAM field to MOOSE variable
  virtual void transferVariable() override;

  // returns the name of the foam variable this object shadows
  virtual std::string foamVariable() const { return _foam_variable; };

protected:
  // variable name or functionObject to be shadowed
  std::string _foam_variable;

  // Pointer to the FoamMesh object
  FoamMesh * _mesh;
};
