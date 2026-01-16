#include "Field.H"
#include "FoamSideIntegratedValue.h"
#include "InputParameters.h"
#include "MooseEnum.h"
#include "MooseTypes.h"
#include "FoamMesh.h"
#include "functionObjects/field/wallShearStress/wallShearStress.H"
#include "volFieldsFwd.H"

registerMooseObject("hippoApp", FoamSideIntegratedValue);

InputParameters
FoamSideIntegratedValue::validParams()
{
  MooseEnum components("x y z normal magnitude", "magnitude");
  auto params = FoamSidePostprocessor::validParams();
  params.addClassDescription(
      "Class that calculates the average or scalar on a OpenFOAM boundary patch.");
  params.addRequiredParam<std::string>(
      "foam_variable", "Foam variable or function object to be averaged over a boundary patch.");
  params.addParam<MooseEnum>(
      "component", components, "If foam variable is a vector, which component to output");
  return params;
}

FoamSideIntegratedValue::FoamSideIntegratedValue(const InputParameters & params)
  : FoamSidePostprocessor(params),
    _value(0.),
    _foam_variable(params.get<std::string>("foam_variable")),
    _is_vector(false),
    _function_object(nullptr)
{
  // Create function object if the foam variable matches one of the
  // available function objects
  if (_pp_function_objects.find(_foam_variable) != _pp_function_objects.items().end())
    createFunctionObject();

  // determine if this is a vector scalar, ahead of computation
  if (_foam_mesh->foundObject<Foam::volVectorField>(_foam_variable))
    _is_vector = true;
  else if (!_foam_mesh->foundObject<Foam::volScalarField>(_foam_variable))
    mooseError("No Foam scalar or function object called '", _foam_variable, "'.");
}

void
FoamSideIntegratedValue::createFunctionObject()
{
  auto fo_dict =
      _foam_mesh->time().controlDict().lookupOrDefault(_foam_variable, Foam::dictionary());

  Foam::wordList patch_names(blocks().begin(), blocks().end());

  fo_dict.set("patches", patch_names);
  fo_dict.set("writeToFile", false);

  if (_foam_variable == "wallHeatFlux")
  {
    _function_object = static_cast<Foam::functionObject *>(
        new Foam::functionObjects::wallHeatFlux("wallHeatFlux", _foam_mesh->time(), fo_dict));
  }
  else if (_foam_variable == "wallShearStress")
  {
    _function_object = static_cast<Foam::functionObject *>(
        new Foam::functionObjects::wallShearStress("wallShearStress", _foam_mesh->time(), fo_dict));
  }

  _function_object->execute();
}

void
FoamSideIntegratedValue::compute()
{
  if (_function_object)
    _function_object->execute();

  _value = 0.;
  // loop over boundary ids
  for (auto & boundary : _boundary)
  {
    auto & areas = _foam_mesh->boundary()[boundary].magSf();
    Foam::Field<double> var_array;

    if (_is_vector)
    {
      // get vector data associated with the boundary
      auto & vec_data =
          _foam_mesh->boundary()[boundary].lookupPatchField<Foam::volVectorField, double>(
              _foam_variable);

      // get the component specified in parameters and get the
      // component of the vector in that direction
      auto components = parameters().get<MooseEnum>("component");
      if (components == "normal")
      {
        auto && normals = _foam_mesh->boundary()[boundary].nf();
        var_array = normals & vec_data;
      }
      else if (components == "magnitude")
        var_array = Foam::mag(vec_data);
      else
        var_array = vec_data.component(int(components));
    }
    else
    {
      var_array = _foam_mesh->boundary()[boundary].lookupPatchField<Foam::volScalarField, double>(
          _foam_variable);
    }

    // Integrate
    for (int i = 0; i < var_array.size(); ++i)
    {
      _value += var_array[i] * areas[i];
    }
  }

  // sum over ranks
  gatherSum(_value);
}

PostprocessorValue
FoamSideIntegratedValue::getValue() const
{
  return _value;
}
