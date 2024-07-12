#pragma once
#include "FoamInterface.h"
#include "fvCFD_moose.h"
#include "PrimitivePatchInterpolation.H"
#include <cassert>
#include <map>
#include <mpi.h>

/*
 * Where the "generic" openFoam state lives mesh, runtime, args and decomposition info
 */

namespace Hippo

{
struct checkedArgList
{
  Foam::argList args;
  checkedArgList(int argc, char ** argv, MPI_Comm const & comm) : args{argc, argv, (void *)&comm}
  {
    int rank;
    MPI_Comm_rank(comm, &rank);
    if (!args.checkRootCase())
    {
      std::cout << "Failed root case check " << rank << std::endl;
    }
  }
};

struct EnvImpl
{
  checkedArgList _args;
  Foam::Time _runtime;
  Foam::fvMesh _mesh;
  Foam::labelList _pointToGlobal;
  Foam::labelList _uniquePoints;
  Foam::autoPtr<Foam::globalIndex> _globalIndex;
  // Not really sure this is the best place for this but keeps any interpolator
  using Interpolator = Foam::PrimitivePatchInterpolation<Foam::polyPatch>;
  using InterpolatorPtr = std::unique_ptr<Interpolator>;
  std::map<int, InterpolatorPtr> _interpolator;
  EnvImpl(int argc, char ** argv, MPI_Comm const & comm)
    : _args{argc, argv, comm},
      _runtime{Foam::Time::controlDictName, _args.args},
      _mesh{Foam::IOobject(
          Foam::fvMesh::defaultRegion, _runtime.timeName(), _runtime, Foam::IOobject::MUST_READ)}
  {
  }

  double getDT() { return _runtime.deltaT().value(); }
  void setDT(double dt) { _runtime.setDeltaTNoAdjust(dt); }
  double getBeginT() { return _runtime.beginTime().value(); }
  double getEndT() { return _runtime.endTime().value(); }

  Foam::polyPatch const & getPatch(std::string const & patch_name)
  {
    return _mesh.boundaryMesh()[patch_name];
  }

  Foam::polyPatch const & getPatch(int const patch_id) { return _mesh.boundaryMesh()[patch_id]; }

  int getPatchID(std::string const & patch_name)
  {
    auto id = _mesh.boundaryMesh().findPatchID(patch_name);
    assert(id != -1 && "Patch name does not exist");
    return id;
  }

  int getComm() { return _mesh.comm(); }
  void calcGlobalData()
  {
    _globalIndex = _mesh.globalData().mergePoints(_pointToGlobal, _uniquePoints);
  }

  Foam::globalIndex const & globalPointNumbering()
  {
    return _mesh.globalData().globalPointNumbering();
  }

#if 0
  auto
  getPatchInterpolator(int patch_id)
  {
    return Foam::PrimitivePatchInterpolation(patch_id);
  }

  auto
  getPatchInterpolator(std::string const & patch_name)
  {
    auto patch_id = getPatchID(patch_name)
    return Foam::PrimitivePatchInterpolation(patch_id);
  }
#endif

  Foam::Time & getRuntime() { return _runtime; }
  Foam::argList & getArglist() { return _args.args; }
  Foam::fvMesh & getMesh() { return _mesh; }
};
}
// Local Variables:
// mode: c++
// End:
