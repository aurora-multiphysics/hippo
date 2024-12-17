#pragma once

#include <fvCFD_moose.h>

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

  size_t append_patch_temperatures(int patch_id, std::vector<double> & foam_t) { return 0; }
  size_t patch_size(int patch_id) { return 0; }
  void set_patch_temperatures(int patch_id, const std::vector<double> & moose_t){};
  void set_patch_negative_heat_flux(int patch_id, std::vector<double> & negative_hf){};

private:
  Foam::solver * _solver;
};

}
