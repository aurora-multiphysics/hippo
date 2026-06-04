#pragma once

#include "FoamBCBase.h"
#include "InputParameters.h"
#include <functional>

class FoamVariableBCBase : public FoamBCBase
{
public:
  static InputParameters validParams();

  explicit FoamVariableBCBase(const InputParameters & params);

  // returns the moose AuxVariable imposed on OpenFOAM
  VariableName mooseVariable() const { return _moose_var->get().name(); }

  virtual void initialSetup() override;

  virtual BCInfoTableRow getInfoRow() const override;

protected:
  // Get the value of the MOOSE variable at an element
  Real variableValueAtElement(const libMesh::Elem & elem) const;

  // Get the data vector of the MOOSE field on a subdomain
  std::vector<Real> getMooseVariableArray(int subdomainId) const;

  // Pointer to Moose variable used to impose BC
  std::optional<std::reference_wrapper<MooseVariableFieldBase>> _moose_var;
};
