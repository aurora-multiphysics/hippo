#include "FoamInterfaceImpl.h"
#include "FoamInterface.h"
#include "ArgsUtil.h"

#include <word.H>
#include <functionObjects/field/wallHeatFlux/wallHeatFlux.H>

#include <cstdio>

namespace Hippo
{
namespace
{
constexpr auto FOAM_PARALLEL_FLAG = "-parallel";

/** Check if OpenFOAM's parallel flag needs to be added to the CLI arguments.
 *
 * Return true if the given command line arguments do not contain the parallel flag and the MPI
 * communicator's size is greater than 1.
 */
bool
foamParallelFlagRequired(std::vector<std::string> const & foam_args, MPI_Comm const & comm)
{
  int comm_size{0};
  MPI_Comm_size(comm, &comm_size);
  return (comm_size > 1 &&
          std::find(foam_args.begin(), foam_args.end(), FOAM_PARALLEL_FLAG) == foam_args.end());
}
}

FoamInterface::FoamInterface(std::vector<std::string> const & foam_args, MPI_Comm const & comm)
{
  auto cargs = cArgs("hippo");
  for (auto const & a : foam_args)
  {
    cargs.push_arg(a);
  }

  // Automatically pass the '-parallel' argument to OpenFOAM if we have more than one MPI rank.
  if (foamParallelFlagRequired(foam_args, comm))
  {
    cargs.push_arg(FOAM_PARALLEL_FLAG);
  }

  _impl = std::make_unique<Hippo::EnvImpl>(cargs.get_argc(), cargs.get_argv().data(), comm);
}

FoamInterface *
FoamInterface::getInstance(std::vector<std::string> const & foam_args, MPI_Comm const & comm)
{
  static FoamInterface instance(foam_args, comm);
  return &instance;
}

void
FoamInterface::dropInstance()
{
  _impl.reset();
}

double
FoamInterface::getTimeDelta()
{
  return _impl->getDT();
}

void
FoamInterface::setCurrentTime(double t)
{
  _impl->setCurrentTime(t);
}

void
FoamInterface::setTimeDelta(double dt)
{
  _impl->setDT(dt);
}

double
FoamInterface::getBeginTime()
{
  return _impl->getBeginT();
}

double
FoamInterface::getEndTime()
{
  return _impl->getEndT();
}

void
FoamInterface::setEndTime(double t)
{
  _impl->setEndT(t);
}

Foam::polyPatch const &
FoamInterface::getPatch(std::string const & patch_name)
{
  return _impl->getPatch(patch_name);
}

Foam::polyPatch const &
FoamInterface::getPatch(int patch_id)
{
  return _impl->getPatch(patch_id);
}

int
FoamInterface::getPatchID(std::string const & patch_name)
{
  return _impl->getPatchID(patch_name);
}

void
FoamInterface::calcGlobalData()
{
  _impl->calcGlobalData();
}

Foam::labelList const &
FoamInterface::pointToGlobal()
{
  return _impl->_pointToGlobal;
}

Foam::labelList const &
FoamInterface::uniquePoints()
{
  return _impl->_uniquePoints;
}

Foam::autoPtr<Foam::globalIndex> const &
FoamInterface::globalIndex()
{
  return _impl->_globalIndex;
}

Foam::globalIndex const &
FoamInterface::globalPointNumbering()
{
  return _impl->globalPointNumbering();
}

Foam::fvMesh &
FoamInterface::getMesh()
{
  return _impl->_mesh;
}

Foam::Time &
FoamInterface::getRuntime()
{
  return _impl->_runtime;
}

Foam::argList &
FoamInterface::getArglist()
{
  return _impl->_args.args;
}

void
FoamInterface::write()
{
  _impl->getMesh().write();
}

std::size_t
FoamInterface::getWallHeatFlux(std::vector<double> & fill_vector, const Foam::label patch_id)
{
  static const Foam::word WALL_HEAT_FLUX = "wallHeatFlux";

  // Calculate the wall heat flux for the given patch.
  auto & impl = getImpl();
  auto dict = impl->_runtime.controlDict();
  auto whf_dict = impl->_runtime.controlDict().lookupOrDefault(WALL_HEAT_FLUX, Foam::dictionary());
  auto patch_name = getMesh().boundaryMesh()[patch_id].name();
  whf_dict.set("patches", Foam::wordList({patch_name}));
  whf_dict.set("writeToFile", false);
  Foam::functionObjects::wallHeatFlux wall_flux(WALL_HEAT_FLUX, impl->getRuntime(), whf_dict);
  wall_flux.execute();

  // Fetch the heat flux and copy it into the given vector.
  auto patch = impl->getPatch(patch_id);
  auto wall_heat_flux = impl->_mesh.lookupObject<Foam::volScalarField>(WALL_HEAT_FLUX);
  auto & hf_bf = wall_heat_flux.boundaryField();
  for (const auto v : hf_bf[patch.index()])
  {
    fill_vector.emplace_back(v);
  }
  return hf_bf[patch.index()].size();
}
}
