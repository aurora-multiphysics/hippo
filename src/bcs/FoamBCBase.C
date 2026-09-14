
#include "FoamBCBase.h"
#include "FoamProblem.h"
#include "HippoObject.h"

#include <Coupleable.h>
#include <InputParameters.h>
#include <MooseError.h>
#include <MooseTypes.h>
#include <MooseVariableFieldBase.h>
#include <Registry.h>
#include <algorithm>
#include <basicThermo.H>

#include <vector>
#include <volFieldsFwd.H>

InputParameters
FoamBCBase::validParams()
{
  InputParameters params = HippoObject::validParams();
  params.addRequiredParam<std::string>("foam_variable",
                                       "Name of a Foam field. e.g. T (temperature) U (velocity).");
  params.addParam<std::vector<SubdomainName>>("boundary",
                                              "Boundaries that the boundary condition applies to.");
  params.addParam<Real>(
      "relaxation_factor", 1., "Relaxation factor for applying boundary conditions.");
  params.registerSystemAttributeName("FoamBC");
  params.registerBase("FoamBC");

  return params;
}

FoamBCBase::FoamBCBase(const InputParameters & params)
  : HippoObject(params),
    Coupleable(this, false),
    _foam_variable(params.isParamValid("foam_variable") ? params.get<std::string>("foam_variable")
                                                        : std::string()),
    _boundary(params.get<std::vector<SubdomainName>>("boundary")),
    _patch_replaced(false),
    _relaxation_factor(getParam<Real>("relaxation_factor"))
{
  // check that the foam variable exists
  if (!params.isPrivate("foam_variable") &&
      !getFoamMesh().foamHasObject<Foam::volScalarField>(_foam_variable) &&
      !getFoamMesh().foamHasObject<Foam::volVectorField>(_foam_variable))
    mooseError("There is no OpenFOAM field named '", _foam_variable, "'");

  // check that the boundary is in the FoamMesh
  auto all_subdomain_names = getFoamMesh().getSubdomainNames(getFoamMesh().getSubdomainList());
  for (auto subdomain : _boundary)
  {
    auto it = std::find(all_subdomain_names.begin(), all_subdomain_names.end(), subdomain);
    if (it == all_subdomain_names.end())
      mooseError("Boundary '", subdomain, "' not found in FoamMesh");
  }

  if (_boundary.empty())
    _boundary = all_subdomain_names;
}

void
FoamBCBase::initialSetup()
{
  for (const auto & boundary : _boundary)
  {
    const auto id = getFvMesh().boundary().findIndex(boundary);
    if (id < 0)
      mooseError("Boundary '", boundary, "' not found in OpenFOAM mesh");
    constructFoamPatch(id);
  }

  imposeBoundaryCondition(true);
}

void
FoamBCBase::constructFixedValuePatch(Foam::label id)
{
  Foam::dictionary dict;
  dict.add("type", "fixedValue");
  if (getFoamMesh().foamHasObject<Foam::volScalarField>(_foam_variable))
  {
    dict.add("value", "uniform 0.");
    if (constructFoamFieldPatch<Foam::scalar>(id, dict))
    {
      Foam::dictionary energy_dict;
      energy_dict.add("type", "fixedEnergy");
      energy_dict.add("value", "uniform 0");
      updateEnergyPatch(
          getFvMesh().lookupObject<Foam::volScalarField>(_foam_variable), id, energy_dict);
    }
  }
  else if (getFoamMesh().foamHasObject<Foam::volVectorField>(_foam_variable))
  {
    dict.add("value", "uniform (0. 0. 0.)");
    constructFoamFieldPatch<Foam::vector>(id, dict);
  }
  else
    mooseError("Variable must have type scalar or vector.");
}

void
FoamBCBase::constructFixedGradientPatch(Foam::label id)
{
  Foam::dictionary dict;
  dict.add("type", "fixedGradient");
  if (getFoamMesh().foamHasObject<Foam::volScalarField>(_foam_variable))
  {
    dict.add("gradient", "uniform 0.");
    if (constructFoamFieldPatch<Foam::scalar>(id, dict))
    {
      Foam::dictionary energy_dict;
      energy_dict.add("type", "gradientEnergy");
      energy_dict.add("gradient", "uniform 0");
      energy_dict.add("value", "uniform 0");
      updateEnergyPatch(
          getFvMesh().lookupObject<Foam::volScalarField>(_foam_variable), id, energy_dict);
    }
  }
  else if (getFoamMesh().foamHasObject<Foam::volVectorField>(_foam_variable))
  {
    dict.add("gradient", "uniform (0. 0. 0.)");
    constructFoamFieldPatch<Foam::vector>(id, dict);
  }
  else
    mooseError("Variable must have type scalar or vector.");
}

void
FoamBCBase::updateEnergyPatch(const Foam::volScalarField & var,
                              Foam::label id,
                              const Foam::dictionary & dict)
{
  auto thermos = getFvMesh().lookupClass<Foam::basicThermo>();
  for (const auto & item : thermos)
  {
    auto & thermo = const_cast<Foam::basicThermo &>(*item);
    if (&thermo.T() != &var)
      continue;

    auto & he = thermo.he();
    he.boundaryFieldRef().set(
        id,
        Foam::fvPatchField<Foam::scalar>::New(he.mesh().boundary()[id], he.internalField(), dict));
  }
}
