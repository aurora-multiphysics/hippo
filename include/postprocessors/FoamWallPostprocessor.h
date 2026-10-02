#pragma once

#include "FoamSidePostprocessor.h"
#include "InputParameters.h"
#include "WallQuantitiesBase.h"
#include <memory>
#include <scalarField.H>

/// Base for integrating and averaging wall quantities over selected OpenFOAM boundaries.
class FoamWallPostprocessor : public FoamSidePostprocessor
{
public:
  static InputParameters validParams();

  explicit FoamWallPostprocessor(const InputParameters & params);

  /// Return the stored postprocessor result.
  virtual PostprocessorValue getValue() const override;

protected:
  /// Integrate per-face values over the selected boundaries and sum across MPI ranks.
  Real integrateField();

  /// Compute the area-weighted average over the selected boundaries across all MPI ranks.
  Real averageField() { return integrateField() / getArea(); }

  /// Return the quantity to integrate for each local face of the specified boundary.
  virtual Foam::scalarField wallField(const std::string & boundary) = 0;

  /// Wall quantity provider for the selected fluid model.
  std::unique_ptr<WallQuantitiesBase> _wall_quantities;

  /// Result stored by compute() for reporting through getValue().
  Real _value;
};
