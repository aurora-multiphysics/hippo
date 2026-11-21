#pragma once

#include "FoamBCBase.h"
#include "InputParameters.h"
#include "FoamFieldAccessor.h"
#include <functional>

/// Scalar BC input supplied through registered patch storage or a MOOSE AuxVariable.
class FoamVariableBCBase : public FoamBCBase
{
public:
  static InputParameters validParams();

  explicit FoamVariableBCBase(const InputParameters & params, const FoamBCType bc_type);

  /// Resolve the input AuxVariable and require CONSTANT MONOMIAL values.
  void initialSetup() override;

  /// Register values, gradients or flux storage under this BC name before INITIAL transfers.
  void registerFieldAccessor() override;

  /// Copy AuxVariable input when direct = false, leaving patch evaluation to OpenFOAM.
  void imposeBoundaryCondition() override;

  /// Describe the input as direct storage or the resolved AuxVariable.
  virtual BCInfoTableRow getInfoRow() const override;

protected:
  /// Receive inputs directly in patch storage instead of copying an AuxVariable.
  const bool _direct;
  /// Determines which patch storage the accessor exposes.
  const FoamBCType _bc_type;

  // Get the value of the MOOSE variable at an element
  Real variableValueAtElement(const libMesh::Elem & elem) const;

  // Get the data vector of the MOOSE field on a subdomain
  std::vector<Real> getMooseVariableArray(int subdomainId) const;

  // Pointer to Moose variable used to impose BC
  std::optional<std::reference_wrapper<MooseVariableFieldBase>> _moose_var;
};
