#pragma once

#include "FoamMesh.h"

#include <Coupleable.h>
#include <InputParameters.h>
#include <MooseTypes.h>
#include <MooseVariableFieldBase.h>
#include "HippoObject.h"

typedef std::tuple<std::string, std::string, std::string, std::string, std::string, std::string>
    BCInfoTableRow;

// valid underlying Foam BC types
enum class FoamBCType
{
  fixedValue,
  fixedGradient
};

class FoamBCBase : public HippoObject, public Coupleable
{
public:
  static InputParameters validParams();

  explicit FoamBCBase(const InputParameters & params, const FoamBCType bc_type);

  virtual void imposeBoundaryCondition() = 0;

  // returns foam variable BC applies to
  std::string foamVariable() const { return _foam_variable; };

  // returns the moose AuxVariable imposed on OpenFOAM
  AuxVariableName mooseVariable() const { return _moose_var->name(); }

  // returns the name of the foam boundaries the BC applies to
  std::vector<SubdomainName> boundary() const { return _boundary; };

  virtual void initialSetup() = 0;

  // Add information about BC to table
  virtual BCInfoTableRow getInfoRow() const = 0;

protected:
  // OpenFOAM variable which this BC is to be imposed on
  std::string _foam_variable;

  // Get the value of the MOOSE variable at an element
  Real variableValueAtElement(const libMesh::Elem * elem);

  // Get the data vector of the MOOSE field on a subdomain
  std::vector<Real> getMooseVariableArray(int subdomain_id);

  // Construct boundary patch for scalar fields
  void constructFoamScalarPatch(const std::string & patch_name, const FoamBCType bc_type);

  // Update the energy equation, if the temperature field is updated the energy equation must be
  void
  updateEnergyPatch(const Foam::volScalarField & var, Foam::label id, const FoamBCType bc_type);

  // Construct boundary patch for vector fields
  void constructFoamVectorPatch(const std::string & patch_name, const FoamBCType bc_type);

  // Pointer to Moose variable used to impose BC
  MooseVariableFieldBase * _moose_var;

  // Boundaries that this object applies to
  // TODO: Replace with inherited from BoundaryRestricted once FoamMesh is updated
  std::vector<SubdomainName> _boundary;

  // Records whether the boundary condition type has been replaced
  bool _patch_replaced;
};
