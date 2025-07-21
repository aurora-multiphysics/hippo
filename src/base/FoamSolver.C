#include "FoamSolver.h"

#include <fixedGradientFvPatchFields.H>
#include <functionObjects/field/wallHeatFlux/wallHeatFlux.H>
#include <fvPatchField.H>
#include <pimpleSingleRegionControl.H>
#include <scalarField.H>
#include <unistd.h>
#include <volFieldsFwd.H>

#include <algorithm>
#include <cassert>
#include <iterator>

namespace Foam
{
namespace functionObjects
{
defineTypeNameAndDebug(mooseDeltaT, 0);

}
}

namespace Hippo
{
namespace
{
/**
 * This was copied (with some minor adjustments) from
 * 'applications/solvers/foamRun/setDeltaT.C' OpenFOAM-12 revision
 * 9ec94dd57a8d98c3f3422ce9b2156a8b268bbda6.
 */
void
adjustDeltaT(Foam::Time & runTime, const Foam::solver & solver)
{
  // Update the time-step limited by the solver maxDeltaT
  if (runTime.controlDict().lookupOrDefault("adjustTimeStep", false) && solver.transient())
  {
    const Foam::scalar deltaT = std::min(solver.maxDeltaT(), runTime.functionObjects().maxDeltaT());

    if (deltaT < Foam::rootVGreat)
    {
      runTime.setDeltaT(std::min(Foam::solver::deltaTFactor * runTime.deltaTValue(), deltaT));
      std::cout << "deltaT = " << runTime.deltaTValue() << std::endl;
    }
  }
}

/**
 * This was copied (with some minor adjustments) from
 * 'applications/solvers/foamRun/setDeltaT.C' OpenFOAM-12 revision
 * 9ec94dd57a8d98c3f3422ce9b2156a8b268bbda6.
 */
void
setDeltaT(Foam::Time & runTime, const Foam::solver & solver)
{
  if (runTime.timeIndex() == 0 && runTime.controlDict().lookupOrDefault("adjustTimeStep", false) &&
      solver.transient())
  {
    const Foam::scalar deltaT = std::min(solver.maxDeltaT(), runTime.functionObjects().maxDeltaT());

    if (deltaT < Foam::rootVGreat)
    {
      runTime.setDeltaT(std::min(runTime.deltaTValue(), deltaT));
    }
  }
}
} // namespace

/**
 * This was copied from 'applications/solvers/foamRun/foamRun.C' OpenFOAM-12
 * revision 9ec94dd57a8d98c3f3422ce9b2156a8b268bbda6. Modifications made:
 *   - We already have a solver, mesh, and runtime, so the construction of them
 * was removed.
 *   - The outer pimple-loop was removed so we're only running one timestep at a
 * time.
 *   - Some changes to the logging.
 */
void
FoamSolver::run()
{
  if (_solver == nullptr)
  {
    return;
  }
  auto & time = runTime();
  auto & solver = *_solver;

  // Create the outer PIMPLE loop and control structure
  Foam::pimpleSingleRegionControl pimple(solver.pimple);

  // Set the initial time-step
  setDeltaT(time, solver);

  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

  // Update PIMPLE outer-loop parameters if changed
  pimple.read();

  solver.preSolve();

  // Adjust the time-step according to the solver maxDeltaT
  adjustDeltaT(time, solver);
  time++;

  _data_backup.loadOldFields();

  // TODO: replace std::cout with MOOSE output or a dependency-injected stream.
  std::cout << "Time = " << time.userTimeName() << "\n" << std::endl;

  // PIMPLE corrector loop
  while (pimple.loop())
  {
    solver.moveMesh();
    solver.motionCorrector();
    solver.fvModels().correct();
    solver.prePredictor();
    solver.momentumPredictor();
    solver.thermophysicalPredictor();
    solver.pressureCorrector();
    solver.postCorrector();
  }

  solver.postSolve();

  time.write();

  std::cout << "ExecutionTime = " << time.elapsedCpuTime() << " s"
            << "  ClockTime = " << time.elapsedClockTime() << " s"
            << "\n"
            << std::endl;
}

std::size_t
FoamSolver::appendPatchTemperatures(const int patch_id, std::vector<double> & foam_t)
{
  if (!_solver)
  {
    return 0;
  }
  auto & mesh = _solver->mesh;
  auto & temp = mesh.boundary()[patch_id].lookupPatchField<Foam::volScalarField, double>("T");
  std::copy(temp.begin(), temp.end(), std::back_inserter(foam_t));
  return temp.size();
}

std::size_t
FoamSolver::patchSize(int patch_id)
{
  if (!_solver)
  {
    return 0;
  }
  auto & mesh = _solver->mesh;
  return mesh.boundary()[patch_id].size();
}

void
FoamSolver::setPatchTemperatures(const int patch_id, const std::vector<double> & moose_t)
{
  if (!_solver)
  {
    return;
  }
  auto & mesh = _solver->mesh;
  auto & temp = const_cast<Foam::fvPatchField<double> &>(
      mesh.boundary()[patch_id].lookupPatchField<Foam::volScalarField, double>("T"));
  assert(moose_t.size() == static_cast<std::size_t>(temp.size()));
  std::copy(moose_t.begin(), moose_t.end(), temp.begin());
}

void
FoamSolver::setPatchNegativeHeatFlux(const int patch_id, std::vector<double> & negative_hf)
{
  if (!_solver)
  {
    return;
  }
  auto & mesh = _solver->mesh;
  auto & temp = const_cast<Foam::fvPatchField<double> &>(
      mesh.boundary()[patch_id].lookupPatchField<Foam::volScalarField, double>("T"));
  Foam::scalarField & temp_gradient(
      Foam::refCast<Foam::fixedGradientFvPatchScalarField>(temp).gradient());
  auto & thermal_conductivity =
      mesh.boundary()[patch_id].lookupPatchField<Foam::volScalarField, double>("kappa");
  assert(temp_gradient.size() == thermal_conductivity.size());
  for (auto i = 0; i < temp_gradient.size(); ++i)
  {
    temp_gradient[i] = negative_hf[i] / thermal_conductivity[i];
  }
}

std::size_t
FoamSolver::wallHeatFlux(const int patch_id, std::vector<double> & fill_vector)
{
  if (!_solver)
  {
    return 0;
  }

  static const Foam::word WALL_HEAT_FLUX = "wallHeatFlux";

  auto whf_dict =
      _solver->runTime.controlDict().lookupOrDefault(WALL_HEAT_FLUX, Foam::dictionary());
  auto patch = _solver->mesh.boundaryMesh()[patch_id];
  whf_dict.set("patches", Foam::wordList({patch.name()}));
  whf_dict.set("writeToFile", false);
  Foam::functionObjects::wallHeatFlux whf_func(WALL_HEAT_FLUX, _solver->runTime, whf_dict);
  whf_func.execute();

  auto wall_heat_flux = _solver->mesh.lookupObject<Foam::volScalarField>(WALL_HEAT_FLUX);
  auto & whf_boundary = wall_heat_flux.boundaryField()[patch.index()];
  for (const auto value : whf_boundary)
  {
    fill_vector.emplace_back(value);
  }
  return whf_boundary.size();
}

void
FoamSolver::preSolve()
{
  _solver->pimple.readIfModified();
  _solver->preSolve();
}

Foam::scalar
FoamSolver::computeDeltaT()
{
  // This code has been adapted from OpenFOAM's adjustDeltaT to determine the time-step that
  // OpenFOAM will use on the next time step so MOOSE can predict it.
  Foam::scalar deltaT =
      std::min(_solver->maxDeltaT(), _solver->runTime.functionObjects().maxDeltaT());

  if (deltaT < Foam::rootVGreat)
    return std::min(Foam::solver::deltaTFactor * _solver->runTime.deltaTValue(), deltaT);
  return _solver->runTime.deltaTValue();
}

bool
FoamSolver::isDeltaTAdjustable() const
{
  return _solver->runTime.controlDict().lookupOrDefault("adjustTimeStep", false);
}

void
FoamSolver::appendDeltaTFunctionObject(const Foam::scalar & dt)
{
  runTime().functionObjects().append(
      new Foam::functionObjects::mooseDeltaT("Moose time step", runTime(), dt));
}
} // namespace Hippo

FoamDataStore::FoamDataStore(Foam::fvMesh & mesh) : _mesh(mesh) {}

inline int64_t
FoamDataStore::_get_field_size() const
{
  int64_t size = _mesh.nCells();
  for (const auto & patch : _mesh.boundary())
  {
    size += patch.size();
  }
  return size;
}

void
FoamDataStore::storeOneScalarField(const Foam::volScalarField & field)
{
  auto it = _scalar_map[field.name()].begin();
  auto const & values = field.internalField();
  for (auto i = 0; i < _mesh.nCells(); ++i)
  {
    *it = values[i];
    it++;
  }

  for (int patchID = 0; patchID < field.boundaryField().size(); ++patchID)
  {
    auto const & patch_values = field.boundaryField()[patchID];
    for (auto i = 0.; i < patch_values.size(); ++i)
    {
      *it = patch_values[i];
      it++;
    }
  }
}

void
FoamDataStore::storeOneVectorField(const Foam::volVectorField & field)
{
  auto it = _vector_map[field.name()].begin();
  auto const & values = field.internalField();
  for (auto i = 0; i < _mesh.nCells(); ++i)
  {
    *it = values[i];
    it++;
  }

  for (int patchID = 0; patchID < field.boundaryField().size(); ++patchID)
  {
    auto const & patch_values = field.boundaryField()[patchID];
    for (auto i = 0.; i < patch_values.size(); ++i)
    {
      *it = patch_values[i];
      it++;
    }
  }
}

void
FoamDataStore::loadOneScalarField(Foam::volScalarField & field)
{
  if (_scalar_map.find(field.name()) == _scalar_map.end())
    return;

  auto it = _scalar_map[field.name()].begin();
  auto & values = field.internalFieldRef();
  for (auto i = 0; i < _mesh.nCells(); ++i)
  {
    values[i] = *it;
    it++;
  }

  for (int patchID = 0; patchID < field.boundaryField().size(); ++patchID)
  {
    auto & patch_values = field.boundaryFieldRef()[patchID];
    for (auto i = 0.; i < patch_values.size(); ++i)
    {
      patch_values[i] = *it;
      it++;
    }
  }
}

void
FoamDataStore::loadOneVectorField(Foam::volVectorField & field)
{
  if (_vector_map.find(field.name()) == _vector_map.end())
    return;

  auto it = _vector_map[field.name()].begin();
  auto & values = field.internalFieldRef();
  for (auto i = 0; i < _mesh.nCells(); ++i)
  {
    values[i] = *it;
    it++;
  }

  for (int patchID = 0; patchID < field.boundaryField().size(); ++patchID)
  {
    auto & patch_values = field.boundaryFieldRef()[patchID];
    for (auto i = 0.; i < patch_values.size(); ++i)
    {
      patch_values[i] = *it;
      it++;
    }
  }
}

void
FoamDataStore::storeFields()
{

  for (auto & field : _mesh.fields<Foam::volScalarField>())
  {
    _scalar_map[field.name()].resize(_get_field_size());
    std::cout << "Saved scalar fields: " << field.name() << std::endl;
    storeOneScalarField(field);
  }

  for (auto & field : _mesh.fields<Foam::volVectorField>())
  {
    _vector_map[field.name()].resize(_get_field_size());
    std::cout << "Saved vector fields: " << field.name() << std::endl;
    storeOneVectorField(field);
  }
}

void
FoamDataStore::loadCurrentFields()
{
  if (_current_loaded)
    return;

  std::cout << "Time: " << _mesh.time().userTimeValue()
            << ". Time index: " << _mesh.time().timeIndex() << std::endl;
  for (auto & field : _mesh.fields<Foam::volScalarField>())
  {
    if (!field.isOldTime())
    {
      std::cout << "Loading current scalar field: " << field.name() << std::endl;
      loadOneScalarField(field);
    }
  }

  for (auto & field : _mesh.fields<Foam::volVectorField>())
  {
    if (!field.isOldTime())
    {
      std::cout << "Loading current vector field: " << field.name() << std::endl;
      loadOneVectorField(field);
    }
  }
  _current_loaded = true;
}

void
FoamDataStore::loadOldFields()
{
  std::cout << "Loading old fields: " << !_old_loaded << std::endl;
  if (_old_loaded)
    return;

  std::cout << "Time: " << _mesh.time().userTimeValue()
            << ". Time index: " << _mesh.time().timeIndex() << std::endl;
  for (auto & field : _mesh.fields<Foam::volScalarField>())
  {
    if (field.isOldTime())
    {
      std::cout << "Loading old scalar field: " << field.name() << std::endl;
      loadOneScalarField(field);
    }
  }

  for (auto & field : _mesh.fields<Foam::volVectorField>())
  {
    if (field.isOldTime())
    {
      std::cout << "Loading old vector field: " << field.name() << std::endl;
      loadOneVectorField(field);
    }
  }
  _old_loaded = true;
}

void
FoamDataStore::storeTime(Foam::Time & time)
{
  _cur_time.timeIndex = time.findClosestTimeIndex(time.times(), time.userTimeValue() - 1);
  _cur_time.deltaT = time.deltaTValue();
  _cur_time.time = time.userTimeValue();
  printf("Saved time: %d, %lf, %lf.\n", _cur_time.timeIndex, _cur_time.time, _cur_time.deltaT);
  fflush(stdout);
}

void
FoamDataStore::loadTime(Foam::Time & time)
{
  time.setTime(time, _cur_time.timeIndex);
  time.setDeltaTNoAdjust(_cur_time.deltaT);
  time.setTime(_cur_time.time, _cur_time.timeIndex - 1);
  printf("Loaded time: %d, %lf, %lf.\n",
         time.findClosestTimeIndex(time.times(), time.userTimeValue()),
         time.userTimeValue(),
         time.userDeltaTValue());
  fflush(stdout);
}
