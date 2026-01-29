#include "Field.H"
#include "FoamSideAverageValue.h"
#include "InputParameters.h"

registerMooseObject("hippoApp", FoamSideAverageValue);

FoamSideAverageValue::FoamSideAverageValue(const InputParameters & params)
  : FoamSideIntegratedValue(params)
{
}

void
FoamSideAverageValue::compute()
{

  Real value = integrateValue();

  Real area = 0.;
  // loop over boundary ids
  for (auto & boundary : _boundary)
  {
    auto & areas = _foam_mesh->boundary()[boundary].magSf();
    for (int i = 0; i < areas.size(); ++i)
    {
      area += areas[i];
    }
  }
  // sum over ranks
  gatherSum(area);

  // divide by area
  _value = value / area;
}
