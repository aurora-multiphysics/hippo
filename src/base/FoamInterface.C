#include "ArgsUtil.h"
#include "FoamInterface.h"
#include "FoamInterfaceImpl.h"
#include "IOobject.H"
#include "IOstream.H"
#include "PtrList.H"
#include "Vector.H"
#include "dictionary.H"
#include "fileName.H"
#include "functionObjects/field/wallHeatFlux/wallHeatFlux.H"
#include "fvBoundaryMesh.H"
#include "fvCFD_moose.h"
#include "objectRegistry.H"
#include "word.H"
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

fs::path
FoamInterface::currentTimePath() const
{
  return fs::path(_impl->_runtime.timePath());
}

// void
// FoamInterface::write(std::ostream & os)
// {

//   // Foam::wordList names(getMesh().names());
//   // if (!names.empty())
//   // {
//   //   for (const auto & name : names)
//   //   {
//   //     getMesh().points().
//   //   }
//   // }

//   // _impl->getMesh().writeObject(
//   //     getRuntime().writeFormat(), getRuntime().writeVersion(), getRuntime().writeCompression());

//   Foam::IOobject mesh_io("polyMesh",
//                          "backup",
//                          getRuntime().timeName(),
//                          getRuntime(),
//                          Foam::IOobject::NO_READ,
//                          Foam::IOobject::AUTO_WRITE);

//   auto & mesh = getMesh();
//   // auto points = mesh.points().clone();
//   // Foam::Vector<double> points(mesh.points().begin(), mesh.points().end());

//   // Foam::wordList boundary_patch_names;
//   // Foam::PtrList<Foam::dictionary> boundary_dicts;
//   // for (const auto & boundary : mesh.boundary())
//   // {
//   //   boundary_patch_names.append(boundary.name());
//   //   boundary_dicts.append(boundary.)
//   // }

//   Foam::fvMesh backup_mesh(mesh_io, mesh.points(), mesh.faces(), mesh.cells());
//   Foam::List<Foam::polyPatch *> patches;
//   for (auto & boundary : mesh.boundary())
//   {
//     // backup_mesh.addFvPatches({boundary});
//     patches.append(&boundary.patch().write(Ostream &));
//   }
//   backup_mesh.addPatches(patches);

//   // Foam::IOstream io(Foam::IOstream::streamFormat::BINARY,
//   //                   getRuntime().writeVersion(),
//   //                   Foam::IOstream::compressionType::UNCOMPRESSED);
//   // _impl->getMesh().writeObject(Foam::IOstream::streamFormat::BINARY,
//   //                              getRuntime().writeVersion(),
//   //                              Foam::IOstream::compressionType::UNCOMPRESSED);
// }

void
FoamInterface::readTime(const std::string & path)
{
  getRuntime().read();
  // Foam::Time new_time(path, {});
  // getRuntime().setTime(new_time);
}

std::size_t
FoamInterface::getWallHeatFlux(std::vector<double> & fill_vector, const Foam::label patch_id)
{
  static const Foam::word WALL_HEAT_FLUX = "wallHeatFlux";

  auto & impl = getImpl();

  Foam::functionObjects::wallHeatFlux wall_flux(
      WALL_HEAT_FLUX, impl->getRuntime(), impl->_runtime.controlDict());
  wall_flux.execute();

  auto patch = impl->getPatch(patch_id);
  auto wall_heat_flux = impl->_mesh.lookupObject<Foam::volScalarField>(WALL_HEAT_FLUX);
  auto & hf_bf = wall_heat_flux.boundaryField();
  // printf("hf_bf.size()=%d\n", hf_bf.size());
  // printf("hf_bf[patch.index()].size()=%d\n", hf_bf[patch.index()].size());
  // printf("hf_bf=[ ");
  // TODO(hsaunders1904): Copying here is unfortunate. However, I get 0s in some elements if I just
  //  return the `Foam::fvPatchField<double>&` from this function. I'm not sure about the scope of
  //  the objects returned by 'lookupObject', maybe it gets deallocated?
  for (const auto v : hf_bf[patch.index()])
  {
    // printf("%f ", v);
    fill_vector.emplace_back(v);
  }
  // printf("]\n");
  return hf_bf[patch.index()].size();
}

// Foam::fvPatchField<double> const &
// FoamInterface::getWallHeatFlux(const Foam::label patch_id)
// {
//   static const Foam::word WALL_HEAT_FLUX = "wallHeatFlux";

//   auto & impl = getImpl();

//   // TODO:
//   //  Need to work out if it's necessary to recalculate this every time.
//   //  We need to recalculate for every time step, so it doesn't work to check the mesh for
//   //  registered objects and only recalculate in those cases - we just end up with the values
//   //  from the first time step.
//   //  One option is to return the boundary field and let the caller index the patch. This API would
//   //  allow us to only calculate the flux on a specific boundary, though. If I can work out how to
//   //  do that.

//   // We may be able to constrain the patches that the flux is calculated for, by defining
//   // a Foam::dictionary that defines a `wordlist` of patch names.
//   // https://www.openfoam.com/documentation/guides/latest/doc/guide-fos-field-wallHeatFlux.html#sec-fos-field-wallHeatFlux-usage
//   // E.g.:
//   //   auto dict = impl->_runtime.controlDict();
//   //   dict["functions"]["wallHeatFlux"].push_back("fluid_bottom");
//   Foam::functionObjects::wallHeatFlux wall_flux(
//       WALL_HEAT_FLUX, impl->getRuntime(), impl->_runtime.controlDict());
//   wall_flux.execute();

//   auto patch = impl->getPatch(patch_id);
//   auto wall_heat_flux = impl->_mesh.lookupObject<Foam::volScalarField>(WALL_HEAT_FLUX);
//   auto & hf_bf = wall_heat_flux.boundaryField();
//   printf("hf_bf.size()=%d\n", hf_bf.size());
//   printf("hf_bf[patch.index()].size()=%d\n", hf_bf[patch.index()].size());
//   printf("hf_bf=[ ");
//   for (const auto v : hf_bf[patch.index()])
//   {
//     printf("%f ", v);
//   }
//   printf("]\n");
//   return hf_bf[patch.index()];
// }

// Foam::fvPatchField<double> const &
// FoamInterface::getWallHeatFlux(const std::string & patch_name)
// {
//   static const Foam::word WALL_HEAT_FLUX = "wallHeatFlux";

//   auto & impl = getImpl();

//   // TODO:
//   //  Need to work out if it's necessary to recalculate this every time.
//   //  We need to recalculate for every time step, so it doesn't work to check the mesh for
//   //  registered objects and only recalculate in those cases - we just end up with the values
//   //  from the first time step.
//   //  One option is to return the boundary field and let the caller index the patch. This API would
//   //  allow us to only calculate the flux on a specific boundary, though. If I can work out how to
//   //  do that.

//   // We may be able to constrain the patches that the flux is calculated for, by defining
//   // a Foam::dictionary that defines a `wordlist` of patch names.
//   // https://www.openfoam.com/documentation/guides/latest/doc/guide-fos-field-wallHeatFlux.html#sec-fos-field-wallHeatFlux-usage
//   // E.g.:
//   //   auto dict = impl->_runtime.controlDict();
//   //   dict["functions"]["wallHeatFlux"].push_back("fluid_bottom");
//   Foam::functionObjects::wallHeatFlux wall_flux(
//       WALL_HEAT_FLUX, impl->getRuntime(), impl->_runtime.controlDict());
//   wall_flux.execute();

//   auto patch = impl->getPatch(patch_name);
//   auto wall_heat_flux = impl->_mesh.lookupObject<Foam::volScalarField>(WALL_HEAT_FLUX);
//   auto & hf_bf = wall_heat_flux.boundaryField();
//   // printf("hf_bf.size()=%d\n", hf_bf.size());
//   // printf("hf_bf[patch.index()].size()=%d\n", hf_bf[patch.index()].size());
//   // printf("hf_bf=[ ");
//   // for (const auto v : hf_bf[patch.index()])
//   // {
//   //   printf("%f ", v);
//   // }
//   // printf("]\n");

//   return hf_bf[patch.index()];
// }

/**
 * TODO:
 *  Get this to work. At the moment, if I include 'volFields.H' or 'volFieldsFwd.H' I get weird
 *  compilation errors, which means I can't return the boundary type from this function.
 */

// Foam::volScalarField::Boundary const &
// FoamInterface::getWallHeatFlux()
// {
//   static const Foam::word WALL_HEAT_FLUX = "wallHeatFlux";

//   auto & impl = getImpl();

//   // We may be able to constrain the patches that the flux is calculated for by defining
//   // a `Foam::dictionary` that defines a `wordlist` of patch names.
//   // https://www.openfoam.com/documentation/guides/latest/doc/guide-fos-field-wallHeatFlux.html#sec-fos-field-wallHeatFlux-usage
//   Foam::functionObjects::wallHeatFlux wall_flux(
//       WALL_HEAT_FLUX, impl->getRuntime(), impl->_runtime.controlDict());
//   wall_flux.execute();

//   auto wall_heat_flux = impl->_mesh.lookupObject<Foam::volScalarField>(WALL_HEAT_FLUX);
//   return wall_heat_flux.boundaryField();
// }
}
