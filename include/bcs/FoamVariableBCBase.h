#pragma once

#include "FoamBCBase.h"
#include "InputParameters.h"

class FoamVariableBCBase : public FoamBCBase
{
public:
  static InputParameters validParams();

  explicit FoamVariableBCBase(const InputParameters & params);

  // returns the moose AuxVariable imposed on OpenFOAM
  VariableName mooseVariable() const { return _moose_var->name(); }

  virtual void initialSetup();

  virtual void addInfoRow(BCInfoTable table);

protected:
  // Get the value of the MOOSE variable at an element
  Real variableValueAtElement(const libMesh::Elem * elem);

  // Get the data vector of the MOOSE field on a subdomain
  std::vector<Real> getMooseVariableArray(int subdomainId);

  // Pointer to Moose variable used to impose BC
  MooseVariableFieldBase * _moose_var;
};
