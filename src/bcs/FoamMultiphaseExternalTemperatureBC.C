#include "FoamMultiphaseExternalTemperatureBC.h"
#include "hippoMultiphaseExternalTemperatureFvPatchScalarField.h"
#include "phaseSystem.H"
#include "mixedEnergyFvPatchScalarField.H"
#include "OStringStream.H"
#include "IStringStream.H"

registerMooseObject("hippoApp", FoamMultiphaseExternalTemperatureBC);

namespace
{
Foam::phaseSystem &
phaseSystem(Foam::fvMesh & mesh)
{
  if (!mesh.foundObject<Foam::phaseSystem>(Foam::phaseSystem::propertiesName))
    mooseError("FoamMultiphaseExternalTemperatureBC requires an OpenFOAM phaseSystem");
  return mesh.lookupObjectRef<Foam::phaseSystem>(Foam::phaseSystem::propertiesName);
}
}

InputParameters
FoamMultiphaseExternalTemperatureBC::validParams()
{
  auto params = FoamVariableBCBase::validParams();
  params.suppressParameter<std::string>("foam_variable");
  params.addClassDescription(
      "Transfers total wall heat flux [W/m^2], positive into the fluid, to all phase "
      "temperatures using multiphaseExternalTemperature and its wall-boiling coupling. "
      "Temperature and energy patches are converted automatically when required.");
  return params;
}

FoamMultiphaseExternalTemperatureBC::FoamMultiphaseExternalTemperatureBC(
    const InputParameters & params)
  : FoamVariableBCBase(params), _phases(phaseSystem(getFvMesh()))
{
  if (_phases.phases().empty())
    mooseError("FoamMultiphaseExternalTemperatureBC found no phases");
}

std::vector<std::string>
FoamMultiphaseExternalTemperatureBC::foamVariables() const
{
  std::vector<std::string> fields;
  for (const auto & phase : _phases.phases())
    fields.push_back(phase.thermo().T().name());
  return fields;
}

void
FoamMultiphaseExternalTemperatureBC::constructFoamPatch(Foam::label id)
{
  for (auto & phase : _phases.phases())
  {
    auto & T = phase.thermo().T();
    if (T.boundaryField()[id].type() != "hippoMultiphaseExternalTemperature")
    {
      Foam::OStringStream os;
      if (Foam::isA<Foam::externalTemperatureFvPatchScalarField>(T.boundaryField()[id]))
        // Preserve compatible thermal settings and mixed coefficients.
        T.boundaryField()[id].write(os);
      else
        // Other patch dictionaries may contain unrelated thermal settings.
        Foam::writeEntry(
            os, "value", static_cast<const Foam::scalarField &>(T.boundaryField()[id]));
      Foam::IStringStream is(os.str());
      Foam::dictionary dict(is);
      dict.set("type", "hippoMultiphaseExternalTemperature");
      dict.remove("q");
      dict.remove("Q");
      dict.set("coupledHeatFlux", "uniform 0");
      T.boundaryFieldRef().set(
          id, Foam::fvPatchScalarField::New(T.mesh().boundary()[id], T.internalField(), dict));
      _patch_replaced = true;
    }

    // Repair energy independently, including when the temperature patch already has our type.
    auto & he = phase.thermo().he();
    if (!Foam::isA<Foam::mixedEnergyFvPatchScalarField>(he.boundaryField()[id]))
    {
      auto * patch =
          new Foam::mixedEnergyFvPatchScalarField(he.mesh().boundary()[id], he.internalField());
      patch->operator==(he.boundaryField()[id]);
      patch->refValue() = he.boundaryField()[id];
      patch->valueFraction() = 1;
      he.boundaryFieldRef().set(id, patch);
      _patch_replaced = true;
    }
  }
}

void
FoamMultiphaseExternalTemperatureBC::imposeBoundaryCondition(bool initialisation)
{
  for (const auto subdomain : getFoamMesh().getSubdomainIDs(_boundary))
  {
    const auto input = getMooseVariableArray(subdomain);
    for (auto & phase : _phases.phases())
    {
      auto & patch = Foam::refCast<Foam::hippoMultiphaseExternalTemperatureFvPatchScalarField>(
          phase.thermo().T().boundaryFieldRef()[subdomain]);
      auto & q = patch.coupledHeatFlux();
      if (input.size() != static_cast<std::size_t>(q.size()))
        mooseError("Heat flux size does not match phase patch '", phase.thermo().T().name(), "'");
      updateBC(q, input, initialisation);
    }
  }
}
