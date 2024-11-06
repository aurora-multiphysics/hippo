#pragma once

#include "FoamMesh.h"
#include "FoamInterface.h"
#include "buoyantFoamApp.h"

#include <libmesh/elem.h>
#include <ExternalProblem.h>
#include <MooseTypes.h>
#include <MooseVariableFieldBase.h>
#include <SystemBase.h>

/* Base class for FoamProblems */

class FoamProblem : public ExternalProblem
{
public:
  FoamProblem(InputParameters const & params);
  static InputParameters validParams();
  virtual void externalSolve() override;
  virtual void syncSolutions(Direction /* dir */) override {}
  virtual bool converged() override { return true; }
  virtual void addExternalVariables() override{};
  // Want to be able to share the object from here so we don't
  // have to pass the args around and problem is available in
  // other objects
  Hippo::FoamInterface * shareInterface() { return _interface; }

  using ExternalProblem::mesh;
  virtual FoamMesh const & mesh() const override { return *_foam_mesh; }
  virtual FoamMesh & mesh() override { return *_foam_mesh; }

protected:
  FoamMesh * _foam_mesh = nullptr;
  Hippo::FoamInterface * _interface = nullptr;
  std::string _backup_foam_timestep_dir;
};

/* Specific class to run buoyantFoam problems */

class BuoyantFoamProblem : public FoamProblem
{
public:
  BuoyantFoamProblem(InputParameters const & params);
  static InputParameters validParams();
  virtual void externalSolve();
  virtual void syncSolutions(Direction dir);
  virtual bool converged() { return true; }
  virtual void addExternalVariables();

protected:
  // TODO(hsaunders1904): can we generalise `_app` so we don't need to implement the transfer
  //  for each 'FoamProblem' we implement?
  Hippo::buoyantFoamApp _app;

private:
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
  getConstantMonomialVariableFromParameters(const std::string & variable_name);
};
