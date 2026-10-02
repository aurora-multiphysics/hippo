#include "FoamSideIntegratedFunctionObject.h"
#include "InputParameters.h"
#include "MooseEnum.h"

registerMooseObject("hippoApp", FoamSideIntegratedFunctionObject);

InputParameters
FoamSideIntegratedFunctionObject::validParams()
{
  InputParameters params = FoamSideIntegratedBase::validParams();

  MooseEnum function_objects("wallHeatFlux wallShearStress");
  params.addRequiredParam<MooseEnum>(
      "function_object", function_objects, "OpenFOAM function object");
  params.addClassDescription(
      "Computes the surface integral of the field produced by an OpenFOAM function object over "
      "the selected boundary patches, using the selected component for vector fields.");
  return params;
}

FoamSideIntegratedFunctionObject::FoamSideIntegratedFunctionObject(const InputParameters & params)
  : FoamSideIntegratedBase(params),
    _function_object(createFunctionObject(getParam<MooseEnum>("function_object")))
{
}

std::unique_ptr<Foam::functionObject>
FoamSideIntegratedFunctionObject::createFunctionObject(const std::string & fo_name)
{
  auto fo_dict = getFoamTime().controlDict().lookupOrDefault(fo_name, Foam::dictionary());

  Foam::wordList patch_names(_boundary.begin(), _boundary.end());

  fo_dict.set("patches", patch_names);
  fo_dict.set("writeToFile", false);

  if (fo_name == "wallHeatFlux")
  {
    return std::make_unique<Foam::functionObjects::wallHeatFlux>(
        "wallHeatFlux", getFoamTime(), fo_dict);
  }
  else // wallShearStress
  {
    return std::make_unique<Foam::functionObjects::wallShearStress>(
        "wallShearStress", getFoamTime(), fo_dict);
  }
}

void
FoamSideIntegratedFunctionObject::compute()
{
  _function_object->execute();
  _value = integrateValue(_function_object->name());
}
