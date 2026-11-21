
#include "FoamBCBase.h"
#include "FoamProblem.h"
#include "HippoObject.h"
#include "hippoDiffusionFluxFvPatchScalarField.h"
#include "processorFvPatch.H"

#include <Coupleable.h>
#include <InputParameters.h>
#include <MooseError.h>
#include <MooseTypes.h>
#include <MooseVariableFieldBase.h>
#include <Registry.h>
#include <basicThermo.H>

#include <vector>
#include <set>
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
    case FoamBCType::diffusionFlux:
      return "hippoDiffusionFlux";
    default:
      mooseError("Unhandled (should be impossible)");
  }
}
}

InputParameters
FoamBCBase::validParams()
{
  InputParameters params = HippoObject::validParams();
  params.addRequiredParam<std::string>("foam_variable",
                                       "Name of a Foam field. e.g. T (temperature) U (velocity).");
  params.addParam<std::vector<SubdomainName>>("boundary",
                                              "Boundaries that the boundary condition applies to.");

  params.registerSystemAttributeName("FoamBC");
  params.registerBase("FoamBC");

  return params;
}

FoamBCBase::FoamBCBase(const InputParameters & params, const FoamBCType bc_type)
  : HippoObject(params),
    Coupleable(this, false),
    _foam_variable(params.get<std::string>("foam_variable")),
    _boundary(params.get<std::vector<SubdomainName>>("boundary")),
    _patch_replaced(false)
{
  // check that the foam variable exists
  if (!params.isPrivate("foam_variable") &&
      !getFoamMesh().foamHasObject<Foam::volScalarField>(_foam_variable))
    mooseError("There is no OpenFOAM field named '", _foam_variable, "'");

  const bool direct = isParamValid("direct") && getParam<bool>("direct");
  const auto mirrored = getFoamMesh().getSubdomainNames(getFoamMesh().getSubdomainList());
  if (_boundary.empty())
    _boundary = mirrored;

  std::set<SubdomainName> selected;
  for (const auto & boundary : _boundary)
  {
    if (!selected.insert(boundary).second)
      paramError("boundary", "Patch '", boundary, "' is specified more than once.");
    if (!direct && std::find(mirrored.begin(), mirrored.end(), boundary) == mirrored.end())
      mooseError("Boundary '", boundary, "' not found in FoamMesh");
    const auto id = getFvMesh().boundary().findIndex(boundary);
    if (id < 0)
      paramError("boundary", "Unknown OpenFOAM patch '", boundary, "'.");
    if (direct)
    {
      if (Foam::isA<Foam::processorFvPatch>(getFvMesh().boundary()[id]))
        paramError("boundary", "Direct BCs require physical OpenFOAM patches.");
    }
  }

  for (auto subdomain : _boundary)
  {
    if (getFoamMesh().foamHasObject<Foam::volScalarField>(_foam_variable))
      constructFoamScalarPatch(subdomain, bc_type);
    else if (getFoamMesh().foamHasObject<Foam::volVectorField>(_foam_variable))
      constructFoamVectorPatch(subdomain, bc_type);
    else
      mooseError("Variable must have type scalar or vector.");
  }
}

void
FoamBCBase::constructFoamScalarPatch(const std::string & patch_name, const FoamBCType bc_type)
{
  auto & var = getFvMesh().lookupObjectRef<Foam::volScalarField>(_foam_variable);
  Foam::label id = getFvMesh().boundary().findIndex(patch_name);

  if (bc_type_to_string(bc_type) == var.boundaryField()[id].type())
  {
    // If there is a different diffusivity in OpenFOAM 0/ to that in the hippo input file,
    // override it with the input file
    if (bc_type == FoamBCType::diffusionFlux)
    {
      auto & patch =
          Foam::refCast<Foam::hippoDiffusionFluxFvPatchScalarField>(var.boundaryFieldRef()[id]);
      // Hippo input controls the coefficient, including when reusing a restart patch.
      patch.setDiffusivity(Foam::word(getParam<std::string>("diffusivity")));
    }
    return;
  }

  // Used by getInfoRow to report in the BC table that the patch as been replaced
  _patch_replaced = true;

  Foam::dictionary bcDict;
  bcDict.add("type", bc_type_to_string(bc_type));
  if (bc_type == FoamBCType::diffusionFlux)
  {
    const auto & diffusivity = getParam<std::string>("diffusivity");
    if (!getFvMesh().foundObject<Foam::volScalarField>(diffusivity))
      paramError("diffusivity", "Field '", diffusivity, "' must be an OpenFOAM volScalarField.");
    const auto & coefficient = getFvMesh().lookupObject<Foam::volScalarField>(diffusivity);
    bcDict.add("diffusivity", Foam::word(diffusivity));
    bcDict.add("fluxDimensions", var.dimensions() * coefficient.dimensions() / Foam::dimLength);
    bcDict.add("flux", "uniform 0");
    bcDict.add("value", "uniform 0");
  }
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
          getFvMesh().boundary()[id], var.boundaryField()[id].internalField(), bcDict));

  // If temperature is replaced, internal energy or enthalpy typically needs replacing.
  updateEnergyPatch(var, id, bc_type);
}

void
FoamBCBase::constructFoamVectorPatch(const std::string & patch_name, const FoamBCType bc_type)
{
  auto & var = getFvMesh().lookupObjectRef<Foam::volVectorField>(_foam_variable);
  Foam::label id = getFvMesh().boundary().findIndex(patch_name);

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

  var.boundaryFieldRef().set(id,
                             Foam::fvPatchField<Foam::vector>::New(
                                 getFvMesh().boundary()[id], var.internalField(), bcDict));
}

void
FoamBCBase::updateEnergyPatch(const Foam::volScalarField & var,
                              Foam::label id,
                              const FoamBCType bc_type)
{

  auto thermos = getFvMesh().lookupClass<Foam::basicThermo>();

  for (const auto & item : thermos)
  {
    auto & thermo = const_cast<Foam::basicThermo &>(*item);

    // Only synchronize the thermo associated with this T field
    if (&thermo.T() != &var)
      continue;

    auto & he = thermo.he(); // Returns e or h, depending on thermo configuration

    Foam::dictionary dict;

    if (bc_type == FoamBCType::fixedGradient || bc_type == FoamBCType::diffusionFlux)
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
