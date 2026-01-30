#include "FoamSideIntegratedFunctionObject.h"
#include "InputParameters.h"
#include "MooseEnum.h"

static MooseEnum _pp_function_objects("wallHeatFlux wallShearStress");

registerMooseObject("hippoApp", FoamSideIntegratedFunctionObject);

InputParameters
FoamSideIntegratedFunctionObject::validParams()
{
  InputParameters params = FoamSideIntegratedBase::validParams();
  params.addRequiredParam<MooseEnum>(
      "function_object", _pp_function_objects, "Foam function object");
  return params;
}

FoamSideIntegratedFunctionObject::FoamSideIntegratedFunctionObject(const InputParameters & params)
  : FoamSideIntegratedBase(params),
    _function_object(createFunctionObject(getParam<MooseEnum>("function_object")))
{
}

Foam::functionObject *
FoamSideIntegratedFunctionObject::createFunctionObject(const std::string & fo_name)
{
  auto fo_dict = _foam_mesh->time().controlDict().lookupOrDefault(fo_name, Foam::dictionary());

  Foam::wordList patch_names(_boundary.begin(), _boundary.end());

  fo_dict.set("patches", patch_names);
  fo_dict.set("writeToFile", false);

  if (fo_name == "wallHeatFlux")
  {
    return static_cast<Foam::functionObject *>(
        new Foam::functionObjects::wallHeatFlux("wallHeatFlux", _foam_mesh->time(), fo_dict));
  }
  else // wallShearStress
  {
    return static_cast<Foam::functionObject *>(
        new Foam::functionObjects::wallShearStress("wallShearStress", _foam_mesh->time(), fo_dict));
  }
}

void
FoamSideIntegratedFunctionObject::compute()
{
  _function_object->execute();
  _value = integrateValue(_function_object->name());
}
