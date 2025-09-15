#include "FoamVariableBase.h"
#include "FoamFunctionObject.h"
#include "InputParameters.h"
#include "MooseError.h"
#include "Registry.h"
#include "SystemBase.h"
#include "dictionary.H"
#include "functionObject.H"
// #include "wallHeatFlux.h"
#include <functionObjects/field/wallHeatFlux/wallHeatFlux.H>
#include "volFieldsFwd.H"

registerMooseObject("hippoApp", FoamFunctionObject);

FoamFunctionObject::FoamFunctionObject(const InputParameters & params) : FoamVariableBase(params)
{
  auto & mesh = _mesh->fvMesh();
  auto fo_dict = mesh.time().controlDict().lookupOrDefault(_foam_variable, Foam::dictionary());

  std::vector<int> patch_ids{_mesh->getSubdomainList()};
  Foam::wordList patch_names;
  for (auto id : patch_ids)
    patch_names.append(mesh.boundaryMesh()[id].name());

  fo_dict.set("patches", patch_names);
  fo_dict.set("writeToFile", false);

  _shadow_fo = _getFunctionObject(fo_dict);
  _shadow_fo->execute();

  _field_shadow = &_mesh->fvMesh().lookupObject<Foam::volScalarField>(_foam_variable);
}

FoamFunctionObject::~FoamFunctionObject() { delete _shadow_fo; }

Foam::functionObject *
FoamFunctionObject::_getFunctionObject(Foam::dictionary fo_dict)
{
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
  std::cout << _foam_variable << std::endl;
  _shadow_fo->execute();
  FoamVariableBase::transferVariable();
}
