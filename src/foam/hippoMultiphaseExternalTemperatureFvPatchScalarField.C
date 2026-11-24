#include "hippoMultiphaseExternalTemperatureFvPatchScalarField.h"
#include "addToRunTimeSelectionTable.H"
#include "fieldMapper.H"

namespace Foam
{
makePatchTypeField(fvPatchScalarField, hippoMultiphaseExternalTemperatureFvPatchScalarField);

hippoMultiphaseExternalTemperatureFvPatchScalarField::
    hippoMultiphaseExternalTemperatureFvPatchScalarField(
        const fvPatch & p, const DimensionedField<scalar, fvMesh> & iF, const dictionary & dict)
  : multiphaseExternalTemperatureFvPatchScalarField(p, iF, dict),
    coupledHeatFlux_("coupledHeatFlux", dimPower / dimArea, dict, p.size())
{
  if (dict.found("q") || dict.found("Q"))
    FatalIOErrorInFunction(dict) << "Use coupledHeatFlux instead of q or Q" << exit(FatalIOError);
}

hippoMultiphaseExternalTemperatureFvPatchScalarField::
    hippoMultiphaseExternalTemperatureFvPatchScalarField(
        const hippoMultiphaseExternalTemperatureFvPatchScalarField & other,
        const fvPatch & p,
        const DimensionedField<scalar, fvMesh> & iF,
        const fieldMapper & mapper)
  : multiphaseExternalTemperatureFvPatchScalarField(other, p, iF, mapper),
    coupledHeatFlux_(mapper(other.coupledHeatFlux_)())
{
}

hippoMultiphaseExternalTemperatureFvPatchScalarField::
    hippoMultiphaseExternalTemperatureFvPatchScalarField(
        const hippoMultiphaseExternalTemperatureFvPatchScalarField & other,
        const DimensionedField<scalar, fvMesh> & iF)
  : multiphaseExternalTemperatureFvPatchScalarField(other, iF),
    coupledHeatFlux_(other.coupledHeatFlux_)
{
}

void
hippoMultiphaseExternalTemperatureFvPatchScalarField::getKappa(scalarField & kappa,
                                                               tmp<scalarField> & sumKappaTcByDelta,
                                                               tmp<scalarField> & sumKappaByDelta,
                                                               tmp<scalarField> & T,
                                                               tmp<scalarField> & sumq) const
{
  multiphaseExternalTemperatureFvPatchScalarField::getKappa(
      kappa, sumKappaTcByDelta, sumKappaByDelta, T, sumq);
  plusEqOp(sumq, tmp<scalarField>(new scalarField(coupledHeatFlux_)));
}

void
hippoMultiphaseExternalTemperatureFvPatchScalarField::map(const fvPatchScalarField & other,
                                                          const fieldMapper & mapper)
{
  multiphaseExternalTemperatureFvPatchScalarField::map(other, mapper);
  mapper(
      coupledHeatFlux_,
      refCast<const hippoMultiphaseExternalTemperatureFvPatchScalarField>(other).coupledHeatFlux_);
}

void
hippoMultiphaseExternalTemperatureFvPatchScalarField::reset(const fvPatchScalarField & other)
{
  multiphaseExternalTemperatureFvPatchScalarField::reset(other);
  coupledHeatFlux_.reset(
      refCast<const hippoMultiphaseExternalTemperatureFvPatchScalarField>(other).coupledHeatFlux_);
}

void
hippoMultiphaseExternalTemperatureFvPatchScalarField::write(Ostream & os) const
{
  multiphaseExternalTemperatureFvPatchScalarField::write(os);
  writeEntry(os, "coupledHeatFlux", coupledHeatFlux_);
}
}
