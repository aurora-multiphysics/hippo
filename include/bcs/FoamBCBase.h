#pragma once

#include "Coupleable.h"
#include "InputParameters.h"
#include "MooseObject.h"
#include "MooseTypes.h"
#include "MooseVariableFieldBase.h"
#include "FoamMesh.h"

class FoamBCBase : public MooseObject, public Coupleable
{
public:
  static InputParameters validParams();

  explicit FoamBCBase(const InputParameters & params);

  virtual void imposeBoundaryCondition() = 0;

protected:
  const MooseVariableFieldBase & getVariable();

  Real variableValueAtElement(const libMesh::Elem * elem);

  std::vector<Real> getVariableArray(int subdomainId);

  const MooseVariableFieldBase & _v;
  FoamMesh * _mesh;

  // Replace with inherited from BoundaryRestricted once FoamMesh is updated
  std::vector<std::string> _boundary;
};
