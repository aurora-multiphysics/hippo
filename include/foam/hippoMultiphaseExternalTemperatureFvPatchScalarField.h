#pragma once

#include "multiphaseExternalTemperatureFvPatchScalarField.H"

namespace Foam
{
/**
 * Multiphase external-temperature patch driven by a writable heat flux per face.
 *
 * coupledHeatFlux_ stores the total wall heat flux [W/m^2], positive into the fluid.
 * The Hippo BC supplies the same total flux to each phase's
 * temperature patch; the inherited multiphase formulation handles the phase
 * coupling rather than requiring Hippo to prescribe a flux split.
 *
 * getKappa() uses the parent method and then adds coupledHeatFlux_ elementwise * to sumq. The
 * inherited updateCoeffs() uses these quantities to construct the mixed boundary coefficients, and
 * mixed-patch evaluation calculates the boundary temperature. This retains the interface used by
 * mixedEnergy and the wall-boiling model without duplicating their calculations.
 */
class hippoMultiphaseExternalTemperatureFvPatchScalarField
  : public multiphaseExternalTemperatureFvPatchScalarField
{
  scalarField coupledHeatFlux_;

protected:
  /// Retain the parent's thermal contributions and add the coupled flux per face to sumq.
  void getKappa(scalarField & kappa,
                tmp<scalarField> & sumKappaTcByDelta,
                tmp<scalarField> & sumKappaByDelta,
                tmp<scalarField> & T,
                tmp<scalarField> & sumq) const override;

public:
  TypeName("hippoMultiphaseExternalTemperature");

  /// Initialize the inherited thermal settings and read the coupled flux from the dictionary.
  hippoMultiphaseExternalTemperatureFvPatchScalarField(const fvPatch &,
                                                       const DimensionedField<scalar, fvMesh> &,
                                                       const dictionary &);
  /// Construct a patch using the supplied mapper for both inherited state and coupled flux.
  hippoMultiphaseExternalTemperatureFvPatchScalarField(
      const hippoMultiphaseExternalTemperatureFvPatchScalarField &,
      const fvPatch &,
      const DimensionedField<scalar, fvMesh> &,
      const fieldMapper &);
  /// Disallow copying without an explicit internal-field reference.
  hippoMultiphaseExternalTemperatureFvPatchScalarField(
      const hippoMultiphaseExternalTemperatureFvPatchScalarField &) = delete;
  /// Copy the inherited state and coupled flux while binding the supplied internal field.
  hippoMultiphaseExternalTemperatureFvPatchScalarField(
      const hippoMultiphaseExternalTemperatureFvPatchScalarField &,
      const DimensionedField<scalar, fvMesh> &);

  /// Create a copy of this patch bound to the supplied internal field.
  tmp<fvPatchScalarField> clone(const DimensionedField<scalar, fvMesh> & iF) const override
  {
    return tmp<fvPatchScalarField>(
        new hippoMultiphaseExternalTemperatureFvPatchScalarField(*this, iF));
  }

  /// Provide writable access to the coupled heat flux per face.
  scalarField & coupledHeatFlux() { return coupledHeatFlux_; }
  /// Provide read-only access to the coupled heat flux per face.
  const scalarField & coupledHeatFlux() const { return coupledHeatFlux_; }
  /// Map the inherited state and coupled flux from another patch using the supplied face mapper.
  void map(const fvPatchScalarField &, const fieldMapper &) override;
  /// Reset the inherited state and coupled flux from another compatible patch.
  void reset(const fvPatchScalarField &) override;
  /// Serialize the inherited patch settings and coupled heat flux.
  void write(Ostream &) const override;
};
}
