#pragma once

#include "FoamWallPostprocessor.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include <map>
#include <scalarField.H>

/// Area-weighted average of local wall HTCs in W/(m^2 K), using supplied bulk temperatures.
class FoamWallAverageHTC : public FoamWallPostprocessor
{
public:
  static InputParameters validParams();

  FoamWallAverageHTC(const InputParameters & params);

  /// Update the stored result with the area-weighted average HTC.
  void compute() override;

protected:
  /// Return per-face HTCs using this boundary's bulk temperature user object.
  Foam::scalarField wallField(const std::string & boundary) override;

  /// Map each boundary to its bulk temperature user object name.
  std::map<SubdomainName, UserObjectName> _t_bulk_uo_names;
};
