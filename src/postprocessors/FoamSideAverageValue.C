#include "Field.H"
#include "FoamSideAverageValue.h"
#include "FoamSideIntegratedValue.h"
#include "InputParameters.h"

registerMooseObject("hippoApp", FoamSideAverageValue);

FoamSideAverageValue::FoamSideAverageValue(const InputParameters & params)
  : FoamSideIntegratedValue(params)
{
}

void
FoamSideAverageValue::compute()
{

  FoamSideIntegratedValue::compute();

  Real volume = 0.;
  // loop over boundary ids
  for (auto & boundary : _boundary)
  {
    auto & areas = _foam_mesh->boundary()[boundary].magSf();
    for (int i = 0; i < areas.size(); ++i)
    {
      volume += areas[i];
    }
  }
  // sum over ranks
  gatherSum(volume);

  // divide by area
  _value /= volume;
}
