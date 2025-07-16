#pragma once

#include "DataIO.h"
#include "FoamMesh.h"
#include "FoamSolver.h"
#include "TimeState.H"
#include "fvMesh.H"
#include "volFieldsFwd.H"

#include <ExternalProblem.h>
#include <MooseTypes.h>
#include <MooseVariableFieldBase.h>
#include <SystemBase.h>
#include <libmesh/elem.h>
#include <memory>

struct FoamTimeState
{
  Foam::label timeIndex;
  Foam::scalar time;
  Foam::scalar deltaT;
};

class FoamDataStore
{
public:
  FoamDataStore() = delete;
  FoamDataStore(Foam::fvMesh & mesh);
  void storeFields();
  void loadFields();
  void storeTime(Hippo::FoamSolver & solver);
  void loadTime(Hippo::FoamSolver & solver);

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

  friend inline void dataStore(std::ostream & stream, FoamDataStore & s, void * context);
  friend inline void dataLoad(std::istream & stream, FoamDataStore & s, void * context);
};

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

  /// Save the current state of the OpenFOAM solve.
  void saveState();

  /// Load the saved state of the OpenFOAM solve.
  void loadState();

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
  double _curr_time;
  int _curr_time_idx;
  FoamDataStore & _mem_buff;
};
