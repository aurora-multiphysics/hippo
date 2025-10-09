#include "ElementUserObject.h"
#include "FoamSideAverageValue.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "FoamMesh.h"

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
        foam_mesh->fvMesh().boundary()[block].lookupPatchField<Foam::volScalarField, double>(
            _foam_scalar);
    auto & areas = foam_mesh->fvMesh().boundary()[block].magSf();

    for (int i = 0; i < var_array.size(); ++i)
    {
      _value += var_array[i] * areas[i];
      _volume += areas[i];
    }
  }

  gatherSum(_value);
  gatherSum(_volume);
  _value /= _volume;

  std::cout << _value << std::endl;
}

PostprocessorValue
FoamSideAverageValue::getValue() const
{
  return _value;
}
