#pragma once

#include "FoamMesh.h"
#include "FoamInterface.h"
#include "FoamSolver.h"

#include <libmesh/elem.h>
#include <ExternalProblem.h>
#include <MooseTypes.h>
#include <MooseVariableFieldBase.h>
#include <SystemBase.h>

class FoamProblem : public ExternalProblem
{
public:
  FoamProblem(InputParameters const & params);
  static InputParameters validParams();
  virtual void externalSolve() override;
  virtual void syncSolutions(Direction /* dir */) override;
  virtual bool converged(const unsigned int nl_sys_num) override { return true; }
  virtual void addExternalVariables() override{};
  // Want to be able to share the object from here so we don't
  // have to pass the args around and problem is available in
  // other objects
  Hippo::FoamInterface * shareInterface() { return _interface; }

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

protected:
  FoamMesh * _foam_mesh = nullptr;
  Hippo::FoamInterface * _interface = nullptr;
  Hippo::FoamSolver _solver;
};
