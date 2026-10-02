#pragma once

#include "FoamWallPostprocessor.h"
#include "InputParameters.h"
#include <scalarField.H>

/// Area-weighted average wall heat flux in W/m^2, positive into the fluid.
class FoamWallAverageHeatFlux : public FoamWallPostprocessor
{
public:
  static InputParameters validParams();

  FoamWallAverageHeatFlux(const InputParameters & params);

  /// Update the stored result with the area-weighted average wall heat flux.
  void compute() override;

protected:
  /// Return the wall heat flux for each local face of the specified boundary.
  Foam::scalarField wallField(const std::string & boundary) override
  {
    return _wall_quantities->wallHeatFlux(boundary);
  }
};
