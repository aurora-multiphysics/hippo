#pragma once

#include "FoamWallPostprocessor.h"
#include "InputParameters.h"
#include <scalarField.H>

/// Area-weighted average wall temperature in K over the selected OpenFOAM boundaries.
class FoamWallAverageTemperature : public FoamWallPostprocessor
{
public:
  static InputParameters validParams();

  explicit FoamWallAverageTemperature(const InputParameters & params);

  /// Update the stored result with the area-weighted average wall temperature.
  void compute() override;

protected:
  /// Return the wall temperature for each local face of the specified boundary.
  Foam::scalarField wallField(const std::string & boundary) override
  {
    return _wall_quantities->wallTemperature(boundary);
  }
};
