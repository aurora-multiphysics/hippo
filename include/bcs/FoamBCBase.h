#pragma once

#include "FoamMesh.h"

#include <Coupleable.h>
#include <InputParameters.h>
#include <MooseObject.h>
#include <MooseTypes.h>
#include <MooseVariableFieldBase.h>

class FoamBCBase : public MooseObject, public Coupleable
{
public:
  static InputParameters validParams();

  explicit FoamBCBase(const InputParameters & params);

  virtual void imposeBoundaryCondition() = 0;

  // returns foam variable BC applies to
  std::string foamVariable() const { return _foam_variable; };

  // returns the moose AuxVariable imposed on OpenFOAM
  AuxVariableName mooseVariable() const { return _moose_var->name(); }

  // returns the name of the foam boundaries the BC applies to
  std::vector<SubdomainName> boundary() const { return _boundary; };

  virtual void initialSetup();

protected:
  // OpenFOAM variable which this BC is to be imposed on
  std::string _foam_variable;

  // Get the value of the MOOSE variable at an element
  Real variableValueAtElement(const libMesh::Elem * elem);

  // Get the data vector of the MOOSE field on a subdomain
  std::vector<Real> getMooseVariableArray(int subdomain_id);

  // Pointer to Moose variable used to impose BC
  MooseVariableFieldBase * _moose_var;

  // Pointer to the FoamMesh object
  FoamMesh * _mesh;

  // Boundaries that this object applies to
  // TODO: Replace with inherited from BoundaryRestricted once FoamMesh is updated
  std::vector<SubdomainName> _boundary;
};
