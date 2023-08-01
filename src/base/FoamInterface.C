#include "ArgsUtil.h"
#include "FoamInterface.h"
#include "FoamInterfaceImpl.h"
#include "fvCFD_moose.h"

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

Foam::Field<double>
FoamInterface::interpolateFaceToNode(int patch_id, Foam::fvPatchField<double> const & field)
{
  return _impl->interpolateFaceToNode(patch_id, field);
}
}
