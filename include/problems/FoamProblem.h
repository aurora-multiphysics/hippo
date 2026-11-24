#pragma once

#include "FoamMesh.h"
#include "FoamPostprocessorBase.h"
#include "FoamSolver.h"
#include "FoamFieldBase.h"
#include "FoamBCBase.h"
#include "FoamFieldRegistry.h"

#include <ExternalProblem.h>
#include <MooseError.h>
#include <MooseTypes.h>
#include <MooseVariableFieldBase.h>
#include <SystemBase.h>
#include <libmesh/elem.h>
#include <memory>
#include <utility>

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

  /// Retrieve an accessor by its qualified object name.
  template <typename T>
  FoamFieldAccessor<T> & getFieldAccessor(const std::string & name)
  {
    return _fields.getFieldAccessor<T>(name);
  }

  bool hasField(const std::string & name) const { return _fields.contains(name); }

  /// Create a distinct accessor under the exposing object's name, borrowing a volume field.
  template <typename T>
  void registerVolFieldVariable(const std::string & variable, const std::string & field);

  /// Register storage under a qualified name, such as FoamBCs/inlet.
  template <typename T>
  FoamFieldAccessor<T> & registerField(const std::string & name,
                                       std::unique_ptr<FoamFieldAccessor<T>> accessor)
  {
    return _fields.add<T>(name, std::move(accessor));
  }

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

  // check FoamPostprocessors and print summarising table
  void verifyFoamPostprocessors();

  FoamMesh * _foam_mesh = nullptr;
  Hippo::FoamSolver _solver;

  std::vector<FoamFieldBase *> _foam_variables;
  std::vector<FoamBCBase *> _foam_bcs;
  std::vector<FoamPostprocessorBase *> _foam_postprocessor;

  FoamFieldRegistry _fields;
};

template <typename T>
void
FoamProblem::registerVolFieldVariable(const std::string & variable, const std::string & field)
{
  if (hasField("FoamVariables/" + variable))
    mooseError("Field accessor '", variable, "' is already registered.");
  auto & mesh = _foam_mesh->fvMesh();
  if (!mesh.foundObject<Foam::VolField<T>>(field))
    mooseError("OpenFOAM volume field '", field, "' is missing or has an incompatible field type.");

  auto & volume_field = mesh.lookupObjectRef<Foam::VolField<T>>(field);
  registerField<T>("FoamVariables/" + variable,
                   std::make_unique<FoamFieldAccessor<T>>(volume_field));
}
