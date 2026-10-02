#pragma once

#include "FoamWallPostprocessor.h"
#include "InputParameters.h"
#include <scalarField.H>

/// Surface integral of wall heat flux, giving the heat transfer rate in W, positive into the fluid.
class FoamWallHeatFluxIntegral : public FoamWallPostprocessor
{
public:
  static InputParameters validParams();

  FoamWallHeatFluxIntegral(const InputParameters & params);

  /// Update the stored result with the integrated wall heat flux.
  void compute() override;

protected:
  /// Return the wall heat flux for each local face of the specified boundary.
  Foam::scalarField wallField(const std::string & boundary) override
  {
    return _wall_quantities->wallHeatFlux(boundary);
  }
};
