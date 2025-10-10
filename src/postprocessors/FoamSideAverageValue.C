#include "ElementUserObject.h"
#include "FoamSideAverageValue.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "FoamMesh.h"
#include "volFieldsFwd.H"

registerMooseObject("hippoApp", FoamSideAverageValue);

InputParameters
FoamSideAverageValue::validParams()
{
  auto params = FoamSidePostprocessor::validParams();
  params.addClassDescription(
      "Class that calculates the average or scalar on a OpenFOAM boundary patch.");
  params.addRequiredParam<std::string>("foam_scalar",
                                       "Foam variable to be averaged over a boundary patch.");
  return params;
}

FoamSideAverageValue::FoamSideAverageValue(const InputParameters & params)
  : FoamSidePostprocessor(params), _value(0.), _foam_scalar(params.get<std::string>("foam_scalar"))
{
  if (!_foam_mesh->foundObject<Foam::volScalarField>(_foam_scalar))
    mooseError("Foam scalar '", _foam_scalar, "' not found.");
}

void
FoamSideAverageValue::compute()
{
  auto foam_mesh = dynamic_cast<FoamMesh *>(&getSubProblem().mesh());
  _value = 0.;
  _volume = 0.;
  for (auto & block : ElementUserObject::blocks())
  {

    auto & var_array =
        _foam_mesh->boundary()[block].lookupPatchField<Foam::volScalarField, double>(_foam_scalar);
    auto & areas = _foam_mesh->boundary()[block].magSf();

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
