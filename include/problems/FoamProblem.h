#pragma once

#include "FoamMesh.h"
#include "FoamPostprocessorBase.h"
#include "FoamSolver.h"
#include "FoamVariableField.h"
#include "FoamBCBase.h"

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
  virtual void initialSetup() override;

  using ExternalProblem::mesh;
  virtual FoamMesh const & mesh() const override { return *_foam_mesh; }
  virtual FoamMesh & mesh() override { return *_foam_mesh; }

  enum class SyncVariables
  {
    WallTemperature,
    WallHeatFlux,
    Both
  };

  Hippo::FoamSolver & solver() { return _solver; }

protected:
  // check FoamVariables and print summarising table
  void verifyFoamVariables();

  // check FoamBCs and print summarising table
  void verifyFoamBCs();

  FoamMesh * _foam_mesh = nullptr;
  Hippo::FoamSolver _solver;

  std::vector<FoamVariableField *> _foam_variables;
  std::vector<FoamBCBase *> _foam_bcs;
  std::vector<FoamPostprocessorBase *> _foam_postprocessor;
};
