#include "FoamSideAverageValue.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "UserObject.h"
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
FoamSideAverageValue::initialize()
{
}

void
FoamSideAverageValue::execute()
{
  auto foam_mesh = dynamic_cast<FoamMesh *>(&_mesh);
  auto boundary = blocks()[0];
  auto & var_array =
      foam_mesh->fvMesh().boundary()[boundary].lookupPatchField<Foam::volScalarField, double>(
          _foam_scalar);
  auto & areas = foam_mesh->fvMesh().boundary()[boundary].magSf();

  Real total_area = 0.;
  _value = 0.;

  std::cout << _value << std::endl;
  for (int i = 0; i < var_array.size(); ++i)
  {
    _value += var_array[i] * areas[i];
    total_area += areas[i];
  }

  _value /= total_area;
}

void
FoamSideAverageValue::threadJoin(const UserObject & uo)
{
}

void
FoamSideAverageValue::finalize()
{
  gatherSum(_value);
}

PostprocessorValue
FoamSideAverageValue::getValue() const
{
  return _value;
}
