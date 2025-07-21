#pragma once

#include "DataIO.h"
#include "fvMesh.H"
#include "solver.H"
#include "functionObject.H"

#include <Time.H>
#include <TimeState.H>
#include <filesystem>
#include <vector>
#include <map>

namespace fs = std::filesystem;

namespace Foam
{
namespace functionObjects
{
// Function object to tell OpenFOAM what MOOSE's dt is
class mooseDeltaT : public functionObject
{
private:
  const scalar & dt_;

public:
  TypeName("mooseDeltaT")

      mooseDeltaT(const word & name, const Time & runTime, const scalar & dt)
    : functionObject(name, runTime), dt_(dt)
  {
  }

  wordList fields() const { return wordList::null(); }

  bool executeAtStart() const { return false; }

  bool execute() { return true; }
  bool write() { return true; }
  scalar maxDeltaT() const { return dt_; }
};
}
}

struct FoamTimeState
{
  Foam::label timeIndex;
  Foam::scalar time;
  Foam::scalar deltaT;
};

class FoamSolver;

class FoamDataStore
{
public:
  FoamDataStore() = delete;
  FoamDataStore(Foam::fvMesh & mesh);
  void storeFields();
  void loadCurrentFields();
  void loadOldFields();
  void storeTime(Foam::Time & time);
  void loadTime(Foam::Time & time);
  void invalidateFields()
  {
    _current_loaded = false;
    _old_loaded = false;
  }
  bool checkInternalField(const Foam::volScalarField & field)
  {
    if (_scalar_map.find(field.name()) == _scalar_map.end())
      return true;

    std::vector<Foam::scalar> copy_internal_field(field.internalField().size());
    std::vector<Foam::scalar> internal_field(field.internalField().size());

    std::copy(
        field.internalField().begin(), field.internalField().end(), copy_internal_field.begin());
    std::copy(_scalar_map[field.name()].begin(),
              _scalar_map[field.name()].begin() + field.internalField().size(),
              internal_field.begin());

    return internal_field == copy_internal_field;
  }

private:
  Foam::fvMesh & _mesh;

  inline int64_t _get_field_size() const;
  inline int64_t _get_buffer_size() const;
  void storeOneScalarField(const Foam::volScalarField & field);
  void storeOneVectorField(const Foam::volVectorField & field);

  void loadOneScalarField(Foam::volScalarField & field);
  void loadOneVectorField(Foam::volVectorField & field);

  FoamTimeState _cur_time;
  std::map<std::string, std::vector<Foam::scalar>> _scalar_map;
  std::map<std::string, std::vector<Foam::Vector<Foam::scalar>>> _vector_map;
  bool _current_loaded = true;
  bool _old_loaded = true;
  friend inline void dataStore(std::ostream & stream, FoamDataStore & s, void * context);
  friend inline void dataLoad(std::istream & stream, FoamDataStore & s, void * context);
};

namespace Hippo
{
class FoamSolver
{
public:
  explicit FoamSolver(Foam::solver * solver)
    : _solver(solver), _data_backup(const_cast<Foam::fvMesh &>(_solver->mesh))
  {
  }

  // Run a timestep of the OpenFOAM solver.
  void run();

  // Append temperature values from the OpenFOAM patch (boundary) to the end of
  // the given vector.
  std::size_t appendPatchTemperatures(int patch_id, std::vector<double> & foam_t);
  // Return the number of faces in the given patch (boundary).
  std::size_t patchSize(int patch_id);
  // Set the temperature values of the OpenFOAM patch (boundary).
  void setPatchTemperatures(int patch_id, const std::vector<double> & moose_t);
  // Set the negative heat flux values on the OpenFOAM patch (boundary).
  void setPatchNegativeHeatFlux(int patch_id, std::vector<double> & negative_hf);
  // Get or calculate the wall heat flux for the given patch.
  std::size_t wallHeatFlux(int patch_id, std::vector<double> & fill_vector);
  // Set the solver's time step size.
  void setTimeDelta(double dt) { runTime().setDeltaTNoAdjust(dt); }
  // Get the current time from the solver.
  double currentTime() { return _solver->time().userTimeValue(); }
  // Get the current time index from the solver.
  int currentTimeIdx()
  {
    return _solver->time().findClosestTimeIndex(runTime().times(), currentTime());
  }
  // Set the solver to the given time.
  void setCurrentTime(double time) { runTime().setTime(time, runTime().timeIndex()); }
  void setCurrentTimeIdx(int timeIdx) { runTime().setTime(runTime().userTimeValue(), timeIdx); }
  // Set the time at which the solver should terminate.
  void setEndTime(double time) { runTime().setEndTime(time); }
  // Run the presolve from MOOSE objects.
  void preSolve();
  // Provide access to the openfoam solver.
  Foam::solver & solver() { return *_solver; };
  // Calculate OpenFOAM's time step.
  Foam::scalar computeDeltaT();
  // check whether OpenFOAM has variable time step.
  bool isDeltaTAdjustable() const;
  // creates function object that tells OpenFOAM what MOOSE's
  // time step is.
  void appendDeltaTFunctionObject(const Foam::scalar & dt);
  // get the current deltaT.
  Foam::scalar getTimeDelta() { return runTime().deltaTValue(); }
  // Get the path to the current time directory, this may or may not exist.
  fs::path currentTimePath() const { return fs::path(runTime().timePath()); }
  // Write the current OpenFOAM timestep to its time directory.
  void write() const { _solver->mesh.write(); }
  // Reset the solver to the given time.
  void readTime(const double time)
  {
    auto idx = _solver->time().findClosestTimeIndex(runTime().times(), time);
    runTime().setTime(time, idx);
    runTime().readModifiedObjects();
  }
  void backupData()
  {
    _data_backup.storeTime(runTime());
    _data_backup.storeFields();
  }
  void restoreData()
  {
    _data_backup.loadTime(runTime());
    _data_backup.invalidateFields();
    _data_backup.loadCurrentFields();
  }

private:
  Foam::solver * _solver = nullptr;

  Foam::Time & runTime() { return const_cast<Foam::Time &>(_solver->runTime); }
  const Foam::Time & runTime() const { return const_cast<Foam::Time &>(_solver->runTime); }
  FoamDataStore _data_backup;
};

} // namespace Hippo

template <typename T>
inline void
dataStore(std::ostream & stream, Foam::Vector<T> & s, void * context)
{
  storeHelper(stream, s.x(), context);
  storeHelper(stream, s.y(), context);
  storeHelper(stream, s.z(), context);
}

template <typename T>
inline void
dataLoad(std::ostream & stream, Foam::Vector<T> & s, void * context)
{
  loadHelper(stream, s.x(), context);
  loadHelper(stream, s.y(), context);
  loadHelper(stream, s.z(), context);
}

inline void
dataStore(std::ostream & stream, FoamTimeState & s, void * context)
{
  storeHelper(stream, s.time, context);
  storeHelper(stream, s.deltaT, context);
  storeHelper(stream, s.timeIndex, context);
}

inline void
dataLoad(std::istream & stream, FoamTimeState & s, void * context)
{
  loadHelper(stream, s.time, context);
  loadHelper(stream, s.deltaT, context);
  loadHelper(stream, s.timeIndex, context);
}

inline void
dataStore(std::ostream & stream, FoamDataStore & s, void * context)
{
  storeHelper(stream, s._scalar_map, context);
  storeHelper(stream, s._vector_map, context);
  storeHelper(stream, s._cur_time, context);
}

inline void
dataLoad(std::istream & stream, FoamDataStore & s, void * context)
{
  loadHelper(stream, s._scalar_map, context);
  loadHelper(stream, s._vector_map, context);
  loadHelper(stream, s._cur_time, context);
}
