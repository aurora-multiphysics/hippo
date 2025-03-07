#pragma once

#include "FoamMesh.h"
#include "FoamSolver.h"

#include <ExternalProblem.h>
#include <MooseTypes.h>
#include <MooseVariableFieldBase.h>
#include <SystemBase.h>
#include <libmesh/elem.h>

class FoamProblem : public ExternalProblem
{
public:
  FoamProblem(InputParameters const & params);
  static InputParameters validParams();
  virtual void externalSolve() override;
  virtual void syncSolutions(Direction /* dir */) override;
  virtual bool converged(const unsigned int /* nl_sys_num */) override { return true; }
  virtual void addExternalVariables() override {};

  using ExternalProblem::mesh;
  virtual FoamMesh const & mesh() const override { return *_foam_mesh; }
  virtual FoamMesh & mesh() override { return *_foam_mesh; }

  enum class SyncVariables
  {
    WallTemperature,
    WallHeatFlux,
    Both
  };

  template <SyncVariables sync_vars>
  void syncFromOpenFoam();

  template <SyncVariables sync_vars>
  void syncToOpenFoam();

  MooseVariableFieldBase *
  getConstantMonomialVariableFromParameters(const std::string & parameter_name);

  Hippo::FoamSolver & solver() { return _solver; }

protected:
  FoamMesh * _foam_mesh = nullptr;
  Hippo::FoamSolver _solver;
};
