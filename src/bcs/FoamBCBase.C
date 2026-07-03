
#include "FoamBCBase.h"
#include "FoamProblem.h"
#include "MooseEnum.h"

#include <Coupleable.h>
#include <DimensionedField.H>
#include <InputParameters.h>
#include <MooseError.h>
#include <MooseObject.h>
#include <MooseTypes.h>
#include <MooseVariableFieldBase.h>
#include <Registry.h>
#include <dimensionedScalar.H>
#include <scalar.H>
#include <scalarField.H>
#include <volFieldsFwd.H>
#include <basicThermo.H>

#include <algorithm>
#include <vector>

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

  params.addPrivateParam<std::string>("_foam_bc_type");

  params.registerSystemAttributeName("FoamBC");
  params.registerBase("FoamBC");

  return params;
}

FoamBCBase::FoamBCBase(const InputParameters & params)
  : MooseObject(params),
    Coupleable(this, false),
    _foam_variable(params.get<std::string>("foam_variable")),
    _boundary(params.get<std::vector<SubdomainName>>("boundary"))
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
      constructFoamScalarPatch(subdomain, params.get<std::string>("_foam_bc_type"));
    else if (_mesh->foamHasObject<Foam::volVectorField>(_foam_variable))
      constructFoamVectorPatch(subdomain, params.get<std::string>("_foam_bc_type"));
    else
      mooseError("Variable must have type scalar or vector.");
  }
}

void
FoamBCBase::constructFoamScalarPatch(const std::string & patch_name, const std::string & bc_type)
{
  auto & var = _mesh->fvMesh().lookupObjectRef<Foam::volScalarField>(_foam_variable);
  Foam::label id = var.mesh().boundary().findIndex(patch_name);
  auto & patch = var.mesh().boundary()[id];

  Foam::dictionary bcDict;
  if (bc_type == "fixedGradient")
  {
    bcDict.add("type", "fixedGradient");
    bcDict.add("gradient", "uniform 0.");
  }
  else if (bc_type == "fixedValue")
  {
    bcDict.add("type", "fixedValue");
    bcDict.add("value", "uniform 0.");
  }
  else
  {
    mooseError("Invalid _foam_bc_type");
  }

  var.boundaryFieldRef().set(id,
                             Foam::fvPatchField<Foam::scalar>::New(
                                 patch, var.boundaryField()[id].internalField(), bcDict));

  updateEnergyPatch(patch_name, bc_type);
}

void
FoamBCBase::constructFoamVectorPatch(const std::string & patch_name, const std::string & bc_type)
{
  auto & var = _mesh->fvMesh().lookupObjectRef<Foam::volVectorField>(_foam_variable);

  Foam::label id = var.mesh().boundary().findIndex(patch_name);
  auto & patch = var.mesh().boundary()[id];

  Foam::dictionary bcDict;
  if (bc_type == "fixedGradient")
  {
    bcDict.add("type", "fixedGradient");
    bcDict.add("gradient", "uniform (0. 0. 0.)");
  }
  else if (bc_type == "fixedValue")
  {
    bcDict.add("type", "fixedValue");
    bcDict.add("value", "uniform (0. 0. 0.)");
  }
  else
  {
    mooseError("Invalid _foam_bc_type");
  }
  var.boundaryFieldRef().set(
      id, Foam::fvPatchField<Foam::vector>::New(patch, var.internalField(), bcDict));
}

void
FoamBCBase::updateEnergyPatch(const std::string & patch_name, const std::string & bc_type)
{
  auto & foam_mesh = _mesh->fvMesh();
  auto & var = foam_mesh.lookupObjectRef<Foam::volScalarField>(_foam_variable);
  Foam::label id = foam_mesh.boundary().findIndex(patch_name);

  auto thermos = foam_mesh.lookupClass<Foam::basicThermo>();

  for (const auto & item : thermos)
  {
    auto & thermo = const_cast<Foam::basicThermo &>(*item);

    // Only synchronize the thermo associated with this T field
    if (&thermo.T() != &var)
      continue;

    auto & he = thermo.he(); // Returns e or h, depending on thermo configuration

    Foam::dictionary dict;

    if (bc_type == "fixedGradient")
    {
      dict.add("type", "gradientEnergy");
      dict.add("gradient", "uniform 0");
      dict.add("value", "uniform 0");
    }
    else if (bc_type == "fixedValue")
    {
      dict.add("type", "fixedEnergy");
      dict.add("value", "uniform 0");
    }

    he.boundaryFieldRef().set(
        id,
        Foam::fvPatchField<Foam::scalar>::New(he.mesh().boundary()[id], he.internalField(), dict));
  }
}
