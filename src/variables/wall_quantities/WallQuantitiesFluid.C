#include "MooseError.h"
#include "MooseTypes.h"
#include "WallQuantitiesBase.h"
#include "WallQuantitiesFluid.h"
#include <UList.H>
#include <basicThermo.H>
#include <scalarField.H>
#include <volFieldsFwd.H>
#include <ThermophysicalTransportModel.H>

WallQuantitiesFluid::WallQuantitiesFluid(const MooseObject * moose_object)
  : WallQuantitiesBase(moose_object)
{
}

const std::string &
WallQuantitiesFluid::getTFieldName()
{
  const auto thermos = getFvMesh().lookupClass<Foam::basicThermo>();
  if (thermos.size() != 1)
    mooseError("Simulation should have exactly one thermo object");

  return thermos.begin()()->T().name();
}

Foam::scalarField
WallQuantitiesFluid::wallTemperature(const SubdomainName & boundary)
{
  auto & boundary_temp =
      getFoamPatch(boundary).lookupPatchField<Foam::volScalarField, double>(getTFieldName());
  return boundary_temp;
}

Foam::scalarField
WallQuantitiesFluid::wallHeatFlux(const SubdomainName & boundary)
{
  auto & patch = getFoamPatch(boundary);
  auto & boundary_temp =
      getFoamPatch(boundary).lookupPatchField<Foam::volScalarField, double>(getTFieldName());

  Foam::Field<Foam::scalar> q_w(patch.size(), 0.);
  const Foam::thermophysicalTransportModel & ttm =
      getFvMesh().lookupType<Foam::thermophysicalTransportModel>();

  // use kappaEff as this would also account for turbulence modelling while being the same as
  // molecular in other cases
  const auto & kappaEffbf = ttm.kappaEff(patch.index());
  q_w = kappaEffbf * boundary_temp.snGrad();

  return q_w;
}

Foam::scalarField
WallQuantitiesFluid::internalTemperature(const Foam::labelUList & cells)
{
  Foam::scalarField l_t_adjacent{cells.size()};

  const auto & T = getFvMesh().lookupObject<Foam::volScalarField>(getTFieldName());
  for (int i = 0; i < l_t_adjacent.size(); ++i)
  {
    l_t_adjacent[i] = T[cells[i]];
  }
  return l_t_adjacent;
}
