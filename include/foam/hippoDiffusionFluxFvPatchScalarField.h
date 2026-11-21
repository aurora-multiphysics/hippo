#pragma once

#include "fixedGradientFvPatchFields.H"
#include "dimensionSet.H"

namespace Foam
{
/// A prescribed per-face diffusion flux, positive into the domain.
class hippoDiffusionFluxFvPatchScalarField : public fixedGradientFvPatchScalarField
{
  /// Registered scalar coefficient field used to convert flux to gradient.
  word diffusivity_;
  /// Expected dimensions of the prescribed flux.
  dimensionSet fluxDimensions_;
  /// Per-face flux storage exposed to direct transfers.
  scalarField flux_;

public:
  TypeName("hippoDiffusionFlux");

  hippoDiffusionFluxFvPatchScalarField(const fvPatch &,
                                       const DimensionedField<scalar, fvMesh> &,
                                       const dictionary &);
  hippoDiffusionFluxFvPatchScalarField(const hippoDiffusionFluxFvPatchScalarField &,
                                       const fvPatch &,
                                       const DimensionedField<scalar, fvMesh> &,
                                       const fieldMapper &);
  hippoDiffusionFluxFvPatchScalarField(const hippoDiffusionFluxFvPatchScalarField &) = delete;
  hippoDiffusionFluxFvPatchScalarField(const hippoDiffusionFluxFvPatchScalarField &,
                                       const DimensionedField<scalar, fvMesh> &);

  /// Copy the patch and prescribed flux onto a different internal field.
  tmp<fvPatchScalarField> clone(const DimensionedField<scalar, fvMesh> & field) const override
  {
    return tmp<fvPatchScalarField>(new hippoDiffusionFluxFvPatchScalarField(*this, field));
  }

  /// Return writable prescribed flux storage.
  scalarField & flux() { return flux_; }
  /// Return the prescribed per-face flux.
  const scalarField & flux() const { return flux_; }
  /// Return the registered diffusivity field name.
  const word & diffusivity() const { return diffusivity_; }
  /// Override the coefficient field name without changing prescribed flux values or dimensions.
  void setDiffusivity(const word & name) { diffusivity_ = name; }

  /// Compute the normal gradient from flux and current diffusivity when evaluation is needed.
  void updateCoeffs() override;
  /// Map patch values, gradients and flux onto the new patch geometry.
  void map(const fvPatchScalarField &, const fieldMapper &) override;
  /// Reset patch values, gradients and flux from another patch of this type.
  void reset(const fvPatchScalarField &) override;
  /// Write patch configuration, prescribed flux and gradient for restart.
  void write(Ostream &) const override;
};
}
