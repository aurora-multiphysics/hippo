#pragma once

#include "MooseTypes.h"
#include "FoamMesh.h"
#include "MooseVariableConstMonomial.h"

class FoamVariableBase : public MooseVariableConstMonomial
{
public:
  static InputParameters validParams();

  explicit FoamVariableBase(const InputParameters & params);

  virtual void transferVariable();

protected:
  std::string _foam_variable;
  FoamMesh * _mesh;
};
