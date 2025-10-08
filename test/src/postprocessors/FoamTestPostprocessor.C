#include "FoamTestPostprocessor.h"
#include "Registry.h"

registerMooseObject("hippoApp", FoamTestPostprocessor);

InputParameters
FoamTestPostprocessor::validParams()
{
  return FoamSidePostprocessor::validParams();
}

FoamTestPostprocessor::FoamTestPostprocessor(const InputParameters & params)
  : FoamSidePostprocessor(params), _value(0.)
{
}

void
FoamTestPostprocessor::execute()
{
  _value = 1.;
}

void
FoamTestPostprocessor::initialize()
{
  _value = 0.;
}

void
FoamTestPostprocessor::threadJoin(const UserObject & uo)
{
}

void
FoamTestPostprocessor::finalize()
{
  gatherSum(_value);
}

PostprocessorValue
FoamTestPostprocessor::getValue() const
{
  return _value;
}
