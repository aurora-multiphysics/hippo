#include "FoamVariable.h"
#include "FoamMesh.h"
#include "FoamFunctionObject.h"
#include "InputParameters.h"
#include "Registry.h"
#include "SystemBase.h"
#include "dictionary.H"
#include "functionObject.H"
#include <functionObjects/field/wallHeatFlux/wallHeatFlux.H>

registerMooseObject("hippoApp", FoamFunctionObject);

InputParameters
FoamFunctionObject::validParams()
{
  auto params = FoamScalarVariable::validParams();
  params.addRequiredParam<std::string>("foam_variable", "Function-object field to mirror.");
  return params;
}

FoamFunctionObject::FoamFunctionObject(const InputParameters & params) : FoamScalarVariable(params)
{
  // construct input Foam dictionary for the functionObject
  auto fo_dict = getFoamTime().controlDict().lookupOrDefault(_foam_variable, Foam::dictionary());

  // create patch names where functionObject applies
  // TODO: when volumetric mirror is implemented some of this may need to be
  // put in the _getFunctionObject function.
  auto patch_ids{getFoamMesh().getSubdomainList()};
  Foam::wordList patch_names;
  for (auto id : patch_ids)
    patch_names.append(getFvMesh().boundary()[id].name());

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
        new Foam::functionObjects::wallHeatFlux("wallHeatFlux", getFoamTime(), fo_dict);
    return static_cast<Foam::functionObject *>(whf_func);
  }
  else
  {
    mooseError("Only wallHeatFlux functionObject is currently supported");
  }
}

void
FoamFunctionObject::syncVariable()
{
  // Update the function-object field before optional mirroring.
  _shadow_fo->execute();
  FoamScalarVariable::syncVariable();
}
