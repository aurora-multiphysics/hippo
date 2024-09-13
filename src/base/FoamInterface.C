#include "FoamInterfaceImpl.h"
#include "FoamInterface.h"
#include "ArgsUtil.h"

#include <word.H>
#include <functionObjects/field/wallHeatFlux/wallHeatFlux.H>

#include <cstdio>

namespace Hippo
{
FoamInterface::FoamInterface(std::vector<std::string> const & foam_args, MPI_Comm const & comm)
{
  auto cargs = cArgs("hippo");
  for (auto const & a : foam_args)
  {
    cargs.push_arg(a);
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
FoamInterface::getDT()
{
  return _impl->getDT();
}

void
FoamInterface::setDT(double dt)
{
  _impl->setDT(dt);
}

double
FoamInterface::getBeginT()
{
  return _impl->getBeginT();
}

double
FoamInterface::getEndT()
{
  return _impl->getEndT();
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

  auto & impl = getImpl();

  // TODO:
  //  Need to work out if it's necessary to recalculate this every time.
  //  We need to recalculate for every time step, so it doesn't work to check the mesh for
  //  registered objects and only recalculate in those cases - we just end up with the values
  //  from the first time step.
  //  One option is to return the boundary field and let the caller index the patch. This API would
  //  allow us to only calculate the flux on a specific boundary, though. If I can work out how to
  //  do that.

  // We may be able to constrain the patches that the flux is calculated for, by defining
  // a Foam::dictionary that defines a `wordlist` of patch names.
  // https://www.openfoam.com/documentation/guides/latest/doc/guide-fos-field-wallHeatFlux.html#sec-fos-field-wallHeatFlux-usage
  // E.g.:
  //   auto dict = impl->_runtime.controlDict();
  //   dict["functions"]["wallHeatFlux"].push_back("fluid_bottom");
  Foam::functionObjects::wallHeatFlux wall_flux(
      WALL_HEAT_FLUX, impl->getRuntime(), impl->_runtime.controlDict());
  wall_flux.execute();

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
