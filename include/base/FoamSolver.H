#pragma once

#include "fvCFD_moose.h"

#include <scalar.H>

#include <vector>

namespace Hippo
{

class FoamSolver
{
public:
  explicit FoamSolver(Foam::solver * solver) : _solver(solver) {}

  // Run a timestep of the OpenFOAM solver.
  void run();

  // Append temperature values from the OpenFOAM patch (boundary) to the end of the given vector.
  std::size_t appendPatchTemperatures(int patch_id, std::vector<double> & foam_t);
  // Return the number of faces in the given patch (boundary).
  std::size_t patchSize(int patch_id);
  // Set the temperature values of the OpenFOAM patch (boundary).
  void setPatchTemperatures(int patch_id, const std::vector<double> & moose_t);
  // Set the negative heat flux values on the OpenFOAM patch (boundary).
  void setPatchNegativeHeatFlux(int patch_id, std::vector<double> & negative_hf);

private:
  Foam::solver * _solver = nullptr;
};

}
