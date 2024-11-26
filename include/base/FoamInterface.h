#pragma once

#include <mpi.h>

#include <vector>
#include <string>
#include <memory>

/*
 * In theory the class through which all calls to the OpenFOAM "environment" are made
 */

namespace Foam
{
class polyPatch;
template <typename T>
class List;
typedef int label;
typedef List<label> labelList;
template <typename T>
class autoPtr;
class globalIndex;
class fvMesh;
class Time;
class argList;
template <typename T>
class fvPatchField;
template <typename T>
class Field;
}

namespace Hippo
{

// Against my better judgment I am creating a singleton
// to do the basic initialization of the foam problem
// I am sure there is a better way to do this
// but I need to initialize it before I construct the
// mesh but I may have more than one mesh(is this true?) so can't
// just have it there
// Forward declare the implementation
struct EnvImpl;

class FoamInterface
{
private:
  std::unique_ptr<Hippo::EnvImpl> _impl;
  FoamInterface(std::vector<std::string> const & args, MPI_Comm const & comm);

public:
  FoamInterface(FoamInterface const &) = delete;
  void operator=(FoamInterface const &) = delete;

  static FoamInterface * getInstance(std::vector<std::string> const & args, MPI_Comm const & comm);
  // TODO: this is not good here at least protectct it and only let it be called drom FoamMesh or
  // something
  void dropInstance();
  // TODO: just a stop gap unit I can warp everything up proper
  // to hide all foaminess  inside this class
  std::unique_ptr<EnvImpl> & getImpl() { return _impl; }

  double getTimeDelta();
  void setTimeDelta(double dt);
  double getBeginTime();
  void setCurrentTime(double t);
  double getEndTime();
  void setEndTime(double t);
  Foam::polyPatch const & getPatch(std::string const & patch_name);
  Foam::polyPatch const & getPatch(int patch_id);
  int getPatchID(std::string const & patch_name);
  void calcGlobalData();
  Foam::labelList const & pointToGlobal();
  Foam::labelList const & uniquePoints();
  Foam::autoPtr<Foam::globalIndex> const & globalIndex();
  Foam::globalIndex const & globalPointNumbering();
  Foam::fvPatchField<double> const & getWallHeatFlux(const std::string & patch_name);
  std::size_t getWallHeatFlux(std::vector<double> & fill_vector, Foam::label patch_id);
  Foam::fvPatchField<double> const & getWallHeatFlux(Foam::label patch_id);
  Foam::Time & getRuntime();
  Foam::argList & getArglist();
  Foam::fvMesh & getMesh();
  void write();
};
}
