#include "FoamPostprocessorBase.h"
#include "FoamSideAverageValue.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "UserObject.h"

InputParameters
FoamSideAverageValue::validParams()
{
  auto params = FoamPostprocessorBase::validParams();
  params.addClassDescription(
      "Class that calculates the average or scalar on a OpenFOAM boundary patch.");
  params.addRequiredParam<std::string>("foam_scalar",
                                       "Foam variable to be averaged over a boundary patch.");
  return params;
}

FoamSideAverageValue::FoamSideAverageValue(const InputParameters & params)
  : FoamPostprocessorBase(params), _value(0.), _foam_scalar(params.get<std::string>("foam_scalar"))
{
}

void
FoamSideAverageValue::initialize()
{
  _value = 0.;
}

void
FoamSideAverageValue::execute()
{
  auto & var_array =
      _mesh->fvMesh().boundary()[_boundary].lookupPatchField<Foam::volScalarField, double>(
          _foam_scalar);
  auto & areas = _mesh->fvMesh().boundary()[_boundary].magSf();

  Real total_area = 0.;

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
  const auto & pps = static_cast<const FoamSideAverageValue &>(uo);
  _value += pps._value;
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
