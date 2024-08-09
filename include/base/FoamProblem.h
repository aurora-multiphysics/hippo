#pragma once

#include "ExternalProblem.h"
#include "Transient.h"
#include "FoamMesh.h"
#include "FoamInterface.h"
#include "buoyantFoamApp.h"

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

  static constexpr auto WALL_HEAT_FLUX_VAR = "wall_heat_flux";
  static constexpr auto WALL_TEMP_VAR = "wall_temperature";

  // /**
  //  * Backs up the application to the folder \p folder_base
  //  *
  //  * @return The files that are written in the backup
  //  */
  // std::vector<std::filesystem::path> backup(const std::filesystem::path & folder_base) {}
  // /**
  //  * Backs up the application memory in a Backup.
  //  *
  //  * @return The backup
  //  */
  // std::unique_ptr<Backup> backup() {}

protected:
  FoamMesh * _foam_mesh = nullptr;
  Hippo::FoamInterface * _interface = nullptr;
  unsigned _wall_temperature{0};
  unsigned _wall_heat_flux{0};
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
  Hippo::buoyantFoamApp _app;
};

// Local Variables:
// mode: c++
// End:
