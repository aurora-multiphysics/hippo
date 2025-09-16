#include "FoamVariableBase.h"
#include "FoamFunctionObject.h"
#include "InputParameters.h"
#include "Registry.h"
#include "SystemBase.h"
#include "dictionary.H"
#include "functionObject.H"
// #include "wallHeatFlux.h"
#include <functionObjects/field/wallHeatFlux/wallHeatFlux.H>

registerMooseObject("hippoApp", FoamFunctionObject);

FoamFunctionObject::FoamFunctionObject(const InputParameters & params) : FoamVariableBase(params)
{
  auto & mesh = _mesh->fvMesh();

  // construct input Foam dictionary for the functionObject
  auto fo_dict = mesh.time().controlDict().lookupOrDefault(_foam_variable, Foam::dictionary());

  // create patch names where functionObject applies
  // TODO: when volumetric mirror is implemented some of this may need to be
  // put in the _getFunctionObject function.
  std::vector<int> patch_ids{_mesh->getSubdomainList()};
  Foam::wordList patch_names;
  for (auto id : patch_ids)
    patch_names.append(mesh.boundaryMesh()[id].name());

  fo_dict.set("patches", patch_names);
  fo_dict.set("writeToFile", false);

  // construct functionObject and execute
  _shadow_fo = _getFunctionObject(fo_dict);
  _shadow_fo->execute();
}

Foam::functionObject *
FoamFunctionObject::_getFunctionObject(Foam::dictionary fo_dict)
{
  // Create polymorphic pointer to each type of valid functionObject
  if (_foam_variable == "wallHeatFlux")
  {
    Foam::functionObjects::wallHeatFlux * whf_func =
        new Foam::functionObjects::wallHeatFlux("wallHeatFlux", _mesh->fvMesh().time(), fo_dict);
    return static_cast<Foam::functionObject *>(whf_func);
  }
  else
  {
    mooseError("Only wallHeatFlux functionObject is currently supported");
  }
}

void
FoamFunctionObject::transferVariable()
{
  // execute functionObject before transfer
  _shadow_fo->execute();
  FoamVariableBase::transferVariable();
}
