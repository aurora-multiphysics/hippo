#include "FoamSideAverageValue.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "UserObject.h"
#include "FoamMesh.h"
#include <cstdint>

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
  _value = 0.;
  _volume = 0.;
}

void
FoamSideAverageValue::execute()
{
  auto foam_mesh = dynamic_cast<FoamMesh *>(&_mesh);
  int64_t patch_offset = 0;
  for (auto & subdomain : foam_mesh->meshSubdomains())
    if (subdomain < _current_elem->subdomain_id())
      patch_offset += foam_mesh->getPatchCount(subdomain);

  auto boundary = foam_mesh->getSubdomainName(_current_elem->subdomain_id());
  auto idx = static_cast<int64_t>(_current_elem->id()) -
             static_cast<int64_t>(foam_mesh->rank_element_offset) - patch_offset;

  auto & var_array =
      foam_mesh->fvMesh().boundary()[boundary].lookupPatchField<Foam::volScalarField, double>(
          _foam_scalar);
  auto & areas = foam_mesh->fvMesh().boundary()[boundary].magSf();

  _value += var_array[idx] * areas[idx];
  _volume += areas[idx];
}

void
FoamSideAverageValue::threadJoin(const UserObject & uo)
{
  (void)uo;
}

void
FoamSideAverageValue::finalize()
{
  gatherSum(_volume);
  gatherSum(_value);
  _value /= _volume;
}

PostprocessorValue
FoamSideAverageValue::getValue() const
{
  return _value;
}
