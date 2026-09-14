#pragma once

#include "FoamMesh.h"
#include <volFields.H>

#include <Coupleable.h>
#include <InputParameters.h>
#include <MooseTypes.h>
#include <MooseVariableFieldBase.h>
#include "HippoObject.h"

typedef std::tuple<std::string, std::string, std::string, std::string, std::string, std::string>
    BCInfoTableRow;

class FoamBCBase : public HippoObject, public Coupleable
{
public:
  static InputParameters validParams();

  explicit FoamBCBase(const InputParameters & params);

  virtual void imposeBoundaryCondition(bool initialisation = false) = 0;

  // returns foam variable BC applies to
  std::string foamVariable() const { return _foam_variable; };

  // All fields modified by this BC, available before initialSetup.
  virtual std::vector<std::string> foamVariables() const { return {_foam_variable}; }

  // returns the moose AuxVariable imposed on OpenFOAM
  AuxVariableName mooseVariable() const { return _moose_var->name(); }

  // returns the name of the foam boundaries the BC applies to
  std::vector<SubdomainName> boundary() const { return _boundary; };

  // Overrides must call their parent initialSetup() to construct the Foam patches.
  virtual void initialSetup();

  // Add information about BC to table
  virtual BCInfoTableRow getInfoRow() const = 0;

protected:
  // OpenFOAM variable which this BC is to be imposed on
  std::string _foam_variable;

  // Get the value of the MOOSE variable at an element
  Real variableValueAtElement(const libMesh::Elem * elem);

  // Get the data vector of the MOOSE field on a subdomain
  std::vector<Real> getMooseVariableArray(int subdomain_id);

  // Called by initialSetup after the complete BC object has been constructed.
  virtual void constructFoamPatch(Foam::label patch_id) = 0;

  // Preserve matching types; return true and record replacement when the type changes.
  template <typename Type>
  bool constructFoamFieldPatch(Foam::label patch_id, const Foam::dictionary & dict);

  // Common patch setup, including associated energy patches for temperature fields.
  void constructFixedValuePatch(Foam::label patch_id);
  void constructFixedGradientPatch(Foam::label patch_id);

  // Replace energy patches only for thermos associated with this temperature field.
  void updateEnergyPatch(const Foam::volScalarField & var,
                         Foam::label patch_id,
                         const Foam::dictionary & dict);

  template <typename Type, typename Array>
  void updateBC(Foam::Field<Type> & foam_patch, const Array & input, bool initialisation);

  template <typename Type>
  void updateBC(Foam::Field<Type> & foam_patch, const Real & input, bool initialisation);

  // Pointer to Moose variable used to impose BC
  MooseVariableFieldBase * _moose_var;

  // Boundaries that this object applies to
  // TODO: Replace with inherited from BoundaryRestricted once FoamMesh is updated
  std::vector<SubdomainName> _boundary;

  // Records whether the boundary condition type has been replaced
  bool _patch_replaced;
  const Real _relaxation_factor;
};

template <typename Type>
bool
FoamBCBase::constructFoamFieldPatch(Foam::label patch_id, const Foam::dictionary & dict)
{
  auto & var = getFvMesh().lookupObjectRef<Foam::VolField<Type>>(_foam_variable);
  if (dict.lookup<Foam::word>("type") == var.boundaryField()[patch_id].type())
    return false;

  var.boundaryFieldRef().set(
      patch_id,
      Foam::fvPatchField<Type>::New(getFvMesh().boundary()[patch_id], var.internalField(), dict));
  _patch_replaced = true;
  return true;
}

template <typename Type, typename Array>
void
FoamBCBase::updateBC(Foam::Field<Type> & foam_patch, const Array & input, bool initialisation)
{
  if (_relaxation_factor != 1.0 && !initialisation)
  {
    for (auto i = 0; i < foam_patch.size(); ++i)
    {
      foam_patch[i] = (1. - _relaxation_factor) * foam_patch[i] + _relaxation_factor * input[i];
    }
  }
  else
  {
    std::copy(input.begin(), input.end(), foam_patch.begin());
  }
}

template <typename Type>
void
FoamBCBase::updateBC(Foam::Field<Type> & foam_patch, const Real & input, bool initialisation)
{
  if (_relaxation_factor != 1.0 && !initialisation)
  {
    for (auto i = 0; i < foam_patch.size(); ++i)
    {
      foam_patch[i] = (1. - _relaxation_factor) * foam_patch[i] + _relaxation_factor * input;
    }
  }
  else
  {
    std::fill(foam_patch.begin(), foam_patch.end(), input);
  }
}
