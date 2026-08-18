
#include "FoamBCBase.h"
#include "FoamProblem.h"

#include <Coupleable.h>
#include <InputParameters.h>
#include <MooseError.h>
#include <MooseObject.h>
#include <MooseTypes.h>
#include <MooseVariableFieldBase.h>
#include <Registry.h>
#include <basicThermo.H>

#include <vector>
#include <volFieldsFwd.H>

namespace
{
std::string
bc_type_to_string(FoamBCType const & bc_type)
{
  switch (bc_type)
  {
    case FoamBCType::fixedValue:
      return "fixedValue";
    case FoamBCType::fixedGradient:
      return "fixedGradient";
    default:
      mooseError("Unhandled (should be impossible)");
  }
}
}

InputParameters
FoamBCBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addRequiredParam<std::string>("foam_variable",
                                       "Name of a Foam field. e.g. T (temperature) U (velocity).");
  params.addParam<std::vector<SubdomainName>>("boundary",
                                              "Boundaries that the boundary condition applies to.");
  params.addRequiredParam<std::string>("foam_variable",
                                       "Name of a Foam field. e.g. T (temperature) U (velocity).");

  params.registerSystemAttributeName("FoamBC");
  params.registerBase("FoamBC");

  return params;
}

FoamBCBase::FoamBCBase(const InputParameters & params, const FoamBCType bc_type)
  : MooseObject(params),
    Coupleable(this, false),
    _foam_variable(params.get<std::string>("foam_variable")),
    _boundary(params.get<std::vector<SubdomainName>>("boundary")),
    _patch_replaced(false)
{
  auto * problem = dynamic_cast<FoamProblem *>(&_c_fe_problem);
  if (!problem)
    mooseError("FoamBC system can only be used with FoamProblem");

  _mesh = &problem->mesh();

  // check that the foam variable exists
  if (!params.isPrivate("foam_variable") &&
      !_mesh->foamHasObject<Foam::volScalarField>(_foam_variable))
    mooseError("There is no OpenFOAM field named '", _foam_variable, "'");

  // check that the boundary is in the FoamMesh
  auto all_subdomain_names = _mesh->getSubdomainNames(_mesh->getSubdomainList());
  for (auto subdomain : _boundary)
  {
    auto it = std::find(all_subdomain_names.begin(), all_subdomain_names.end(), subdomain);
    if (it == all_subdomain_names.end())
      mooseError("Boundary '", subdomain, "' not found in FoamMesh");
  }

  if (_boundary.empty())
    _boundary = all_subdomain_names;

  for (auto subdomain : _boundary)
  {
    if (_mesh->foamHasObject<Foam::volScalarField>(_foam_variable))
      constructFoamScalarPatch(subdomain, bc_type);
    else if (_mesh->foamHasObject<Foam::volVectorField>(_foam_variable))
      constructFoamVectorPatch(subdomain, bc_type);
    else
      mooseError("Variable must have type scalar or vector.");
  }
}

void
FoamBCBase::constructFoamScalarPatch(const std::string & patch_name, const FoamBCType bc_type)
{
  auto & foam_mesh = _mesh->fvMesh();
  auto & var = foam_mesh.lookupObjectRef<Foam::volScalarField>(_foam_variable);
  Foam::label id = foam_mesh.boundary().findIndex(patch_name);

  if (bc_type_to_string(bc_type) == var.boundaryField()[id].type())
    return;

  // Used by getInfoRow to report in the BC table that the patch as been replaced
  _patch_replaced = true;

  Foam::dictionary bcDict;
  bcDict.add("type", bc_type_to_string(bc_type));
  if (bc_type == FoamBCType::fixedGradient)
  {
    bcDict.add("gradient", "uniform 0.");
  }
  else if (bc_type == FoamBCType::fixedValue)
  {
    bcDict.add("value", "uniform 0.");
  }

  var.boundaryFieldRef().set(
      id,
      Foam::fvPatchField<Foam::scalar>::New(
          foam_mesh.boundary()[id], var.boundaryField()[id].internalField(), bcDict));

  // If temperature is replaced, internal energy or enthalpy typically needs replacing.
  updateEnergyPatch(var, id, bc_type);
}

void
FoamBCBase::constructFoamVectorPatch(const std::string & patch_name, const FoamBCType bc_type)
{
  auto & foam_mesh = _mesh->fvMesh();
  auto & var = foam_mesh.lookupObjectRef<Foam::volVectorField>(_foam_variable);
  Foam::label id = foam_mesh.boundary().findIndex(patch_name);

  if (bc_type_to_string(bc_type) == var.boundaryField()[id].type())
    return;

  _patch_replaced = true;

  Foam::dictionary bcDict;
  bcDict.add("type", bc_type_to_string(bc_type));
  if (bc_type == FoamBCType::fixedGradient)
  {
    bcDict.add("gradient", "uniform (0. 0. 0.)");
  }
  else if (bc_type == FoamBCType::fixedValue)
  {
    bcDict.add("value", "uniform (0. 0. 0.)");
  }

  var.boundaryFieldRef().set(
      id,
      Foam::fvPatchField<Foam::vector>::New(foam_mesh.boundary()[id], var.internalField(), bcDict));
}

void
FoamBCBase::updateEnergyPatch(const Foam::volScalarField & var,
                              Foam::label id,
                              const FoamBCType bc_type)
{

  auto thermos = var.mesh().lookupClass<Foam::basicThermo>();

  for (const auto & item : thermos)
  {
    auto & thermo = const_cast<Foam::basicThermo &>(*item);

    // Only synchronize the thermo associated with this T field
    if (&thermo.T() != &var)
      continue;

    auto & he = thermo.he(); // Returns e or h, depending on thermo configuration

    Foam::dictionary dict;

    if (bc_type == FoamBCType::fixedGradient)
    {
      dict.add("type", "gradientEnergy");
      dict.add("gradient", "uniform 0");
      dict.add("value", "uniform 0");
    }
    else if (bc_type == FoamBCType::fixedValue)
    {
      dict.add("type", "fixedEnergy");
      dict.add("value", "uniform 0");
    }

    he.boundaryFieldRef().set(
        id,
        Foam::fvPatchField<Foam::scalar>::New(he.mesh().boundary()[id], he.internalField(), dict));
  }
}
