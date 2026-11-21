#include "hippoDiffusionFluxFvPatchScalarField.h"
#include "addToRunTimeSelectionTable.H"
#include "fieldMapper.H"
#include "volFields.H"

namespace Foam
{
makePatchTypeField(fvPatchScalarField, hippoDiffusionFluxFvPatchScalarField);

hippoDiffusionFluxFvPatchScalarField::hippoDiffusionFluxFvPatchScalarField(
    const fvPatch & patch, const DimensionedField<scalar, fvMesh> & field, const dictionary & dict)
  : fixedGradientFvPatchScalarField(patch, field, dict, false),
    diffusivity_(dict.lookupOrDefault<word>("diffusivity", "kappa")),
    fluxDimensions_(dict.lookup("fluxDimensions")),
    flux_("flux", fluxDimensions_, dict, patch.size())
{
  // Preserve restart gradients without looking up diffusivity before it is registered.
  if (dict.found("gradient"))
    gradient() = scalarField("gradient", field.dimensions() / dimLength, dict, patch.size());
  else
    gradient() = scalar(0);

  if (dict.found("value"))
    fvPatchScalarField::operator=(scalarField("value", field.dimensions(), dict, patch.size()));
  else
    fvPatchScalarField::operator=(patchInternalField() + gradient() / patch.deltaCoeffs());
}

hippoDiffusionFluxFvPatchScalarField::hippoDiffusionFluxFvPatchScalarField(
    const hippoDiffusionFluxFvPatchScalarField & other,
    const fvPatch & patch,
    const DimensionedField<scalar, fvMesh> & field,
    const fieldMapper & mapper)
  : fixedGradientFvPatchScalarField(other, patch, field, mapper),
    diffusivity_(other.diffusivity_),
    fluxDimensions_(other.fluxDimensions_),
    flux_(mapper(other.flux_)())
{
}

hippoDiffusionFluxFvPatchScalarField::hippoDiffusionFluxFvPatchScalarField(
    const hippoDiffusionFluxFvPatchScalarField & other,
    const DimensionedField<scalar, fvMesh> & field)
  : fixedGradientFvPatchScalarField(other, field),
    diffusivity_(other.diffusivity_),
    fluxDimensions_(other.fluxDimensions_),
    flux_(other.flux_)
{
}

void
hippoDiffusionFluxFvPatchScalarField::updateCoeffs()
{
  if (updated())
    return;

  const auto & coefficient = db().lookupObject<volScalarField>(diffusivity_);
  const dimensionSet expectedDimensions =
      internalField().dimensions() * coefficient.dimensions() / dimLength;
  if (fluxDimensions_ != expectedDimensions)
    FatalErrorInFunction << "Flux dimensions " << fluxDimensions_ << " on patch " << patch().name()
                         << " for field " << internalField().name()
                         << " do not match expected dimensions " << expectedDimensions
                         << ", using diffusivity field " << diffusivity_ << " with dimensions "
                         << coefficient.dimensions() << exit(FatalError);
  const auto & values = coefficient.boundaryField()[patch().index()];
  forAll(values, i)
  {
    if (values[i] == 0)
      FatalErrorInFunction << "Zero diffusivity on patch " << patch().name() << " at face " << i
                           << exit(FatalError);
  }

  gradient() = flux_ / values;
  fixedGradientFvPatchScalarField::updateCoeffs();
}

void
hippoDiffusionFluxFvPatchScalarField::map(const fvPatchScalarField & other,
                                          const fieldMapper & mapper)
{
  fixedGradientFvPatchScalarField::map(other, mapper);
  mapper(flux_, refCast<const hippoDiffusionFluxFvPatchScalarField>(other).flux_);
}

void
hippoDiffusionFluxFvPatchScalarField::reset(const fvPatchScalarField & other)
{
  fixedGradientFvPatchScalarField::reset(other);
  flux_.reset(refCast<const hippoDiffusionFluxFvPatchScalarField>(other).flux_);
}

void
hippoDiffusionFluxFvPatchScalarField::write(Ostream & os) const
{
  fixedGradientFvPatchScalarField::write(os);
  writeEntry(os, "diffusivity", diffusivity_);
  writeEntry(os, "fluxDimensions", fluxDimensions_);
  writeEntry(os, "flux", flux_);
}
}
