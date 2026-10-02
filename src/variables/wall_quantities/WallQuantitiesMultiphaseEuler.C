#include "InputParameters.h"
#include "MooseObject.h"
#include "MooseTypes.h"
#include "WallQuantitiesMultiphaseEuler.h"
#include <scalarField.H>
#include "fvCFD_moose.h"
#include <wallBoiling.H>

WallQuantitiesMultiphaseEuler::WallQuantitiesMultiphaseEuler(const MooseObject * moose_object)
  : WallQuantitiesBase(moose_object), _phase_system()
{
  if (!getFvMesh().foundObject<Foam::phaseSystem>(Foam::phaseSystem::propertiesName))
    mooseError("No phaseSystem found");

  _phase_system = getFvMesh().lookupObject<Foam::phaseSystem>(Foam::phaseSystem::propertiesName);
}

Foam::scalarField
WallQuantitiesMultiphaseEuler::wallTemperature(const SubdomainName & boundary)
{
  auto & patch = getFoamPatch(boundary);
  Foam::Field<Foam::scalar> Tw(patch.size(), 0.);
  for (const auto & model : _phase_system->get().phases())
  {
    const auto & Tbf =
        patch.lookupPatchField<Foam::volScalarField, double>(model.thermo().T().name());

    const auto & alpha = model.boundaryField()[patch.index()];
    Tw += alpha * Tbf;
  }

  return Tw;
}

Foam::scalarField
WallQuantitiesMultiphaseEuler::wallHeatFlux(const SubdomainName & boundary)
{
  auto & patch = getFoamPatch(boundary);
  Foam::scalarField q_w(patch.size(), 0.);
  // Loop through phases and add contribution from each
  for (const auto & model : _phase_system->get().phases())
  {
    const auto & kappaEffbf = model.kappaEff(patch.index());
    const auto & Tbf =
        patch.lookupPatchField<Foam::volScalarField, double>(model.thermo().T().name());

    const auto & alpha = model.boundaryField()[patch.index()];
    q_w += alpha * kappaEffbf * Tbf.snGrad();
  }

  return q_w;
}

Foam::scalarField
WallQuantitiesMultiphaseEuler::internalTemperature(const Foam::labelUList & cells)
{
  Foam::scalarField l_t_adjacent(cells.size(), 0.);

  for (const auto & model : _phase_system->get().phases())
  {
    const auto & T = model.thermo().T();
    const auto & alpha = model;
    for (int i = 0; i < l_t_adjacent.size(); ++i)
    {
      int idx = cells[i];
      l_t_adjacent[i] += alpha[idx] * T[idx];
    }
  }
  return l_t_adjacent;
}
