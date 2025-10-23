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

  // returns foam variable BC applies to
  std::string foamVariable() const { return _foam_variable; };

  // returns foam variable BC applies to
  std::vector<SubdomainName> boundary() const { return _boundary; };

  // returns MOOSE variable used by BC
  std::string mooseVariable() const { return _v.name(); };

protected:
  // OpenFOAM variable which this BC is to be imposed on
  std::string _foam_variable;

  // Get a constant reference to the underlying MOOSE field
  // in the parameters object
  const MooseVariableFieldBase & getVariable(const InputParameters & params);

  // Get the value of the MOOSE variable at an element
  Real variableValueAtElement(const libMesh::Elem * elem);

  // Get the data vector of the MOOSE field on a subdomain
  std::vector<Real> getMooseVariableArray(int subdomainId);

  // Underlying MOOSE field
  const MooseVariableFieldBase & _v;

  // Pointer to the FoamMesh object
  FoamMesh * _mesh;

  // Boundaries that this object applies to
  // TODO: Replace with inherited from BoundaryRestricted once FoamMesh is updated
  std::vector<SubdomainName> _boundary;
};
