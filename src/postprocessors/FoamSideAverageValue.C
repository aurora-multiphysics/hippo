#include "ElementUserObject.h"
#include "Field.H"
#include "FoamSideAverageValue.h"
#include "InputParameters.h"
#include "MooseEnum.h"
#include "MooseTypes.h"
#include "FoamMesh.h"
#include "functionObjects/field/wallShearStress/wallShearStress.H"
#include "volFieldsFwd.H"

registerMooseObject("hippoApp", FoamSideAverageValue);

InputParameters
FoamSideAverageValue::validParams()
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

FoamSideAverageValue::FoamSideAverageValue(const InputParameters & params)
  : FoamSidePostprocessor(params),
    _value(0.),
    _foam_variable(params.get<std::string>("foam_variable")),
    _is_vector(false),
    _function_object(nullptr)
{
  if (_pp_function_objects.find(_foam_variable) != _pp_function_objects.items().end())
    createFunctionObject();

  if (_foam_mesh->foundObject<Foam::volVectorField>(_foam_variable))
    _is_vector = true;
  else if (!_foam_mesh->foundObject<Foam::volScalarField>(_foam_variable))
    mooseError("No Foam scalar or function object called '", _foam_variable, "'.");
}

void
FoamSideAverageValue::createFunctionObject()
{
  auto fo_dict =
      _foam_mesh->time().controlDict().lookupOrDefault(_foam_variable, Foam::dictionary());

  Foam::wordList patch_names;
  for (auto id : blocks())
    patch_names.append(_foam_mesh->boundaryMesh()[id].name());

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
FoamSideAverageValue::compute()
{
  auto foam_mesh = dynamic_cast<FoamMesh *>(&getSubProblem().mesh());
  _value = 0.;
  _volume = 0.;

  if (_function_object)
    _function_object->execute();

  for (auto & block : ElementUserObject::blocks())
  {
    auto & areas = _foam_mesh->boundary()[block].magSf();
    Foam::Field<double> var_array;

    if (_is_vector)
    {
      auto && normals = _foam_mesh->boundary()[block].nf();
      auto & vec_data =
          _foam_mesh->boundary()[block].lookupPatchField<Foam::volVectorField, double>(
              _foam_variable);

      auto components = parameters().get<MooseEnum>("component");
      if (components == "normal")
        var_array = normals & vec_data;
      else if (components == "magnitude")
        var_array = Foam::mag(vec_data);
      else
        var_array = vec_data.component(int(components));
    }
    else
    {
      var_array = _foam_mesh->boundary()[block].lookupPatchField<Foam::volScalarField, double>(
          _foam_variable);
    }

    for (int i = 0; i < var_array.size(); ++i)
    {
      _value += var_array[i] * areas[i];
      _volume += areas[i];
    }
  }

  gatherSum(_value);
  gatherSum(_volume);
  _value /= _volume;
}

PostprocessorValue
FoamSideAverageValue::getValue() const
{
  return _value;
}
