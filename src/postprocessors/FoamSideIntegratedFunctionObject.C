#include "FoamSideIntegratedBase.h"
#include "FoamSideIntegratedFunctionObject.h"
#include "InputParameters.h"
#include "MooseEnum.h"
#include <volFieldsFwd.H>

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
  : FoamSideIntegratedBase(params), _function_object()
{
  _foam_variable = std::string(getParam<MooseEnum>("function_object"));
  _function_object.reset(createFunctionObject());
}

Foam::functionObject *
FoamSideIntegratedFunctionObject::createFunctionObject()
{
  auto fo_dict =
      _foam_mesh->time().controlDict().lookupOrDefault(_foam_variable, Foam::dictionary());

  Foam::wordList patch_names(blocks().begin(), blocks().end());

  fo_dict.set("patches", patch_names);
  fo_dict.set("writeToFile", false);

  if (_foam_variable == "wallHeatFlux")
  {
    _is_vector = false;
    return static_cast<Foam::functionObject *>(
        new Foam::functionObjects::wallHeatFlux("wallHeatFlux", _foam_mesh->time(), fo_dict));
  }
  else // wallShearStress
  {
    _is_vector = true;
    return static_cast<Foam::functionObject *>(
        new Foam::functionObjects::wallShearStress("wallShearStress", _foam_mesh->time(), fo_dict));
  }
}

Real
FoamSideIntegratedFunctionObject::integrateValue()
{
  _function_object->execute();
  return FoamSideIntegratedBase::integrateValue();
}
