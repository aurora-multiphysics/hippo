#include "ElementUserObject.h"
#include "FoamSideAdvectiveFluxIntegral.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "FoamMesh.h"

registerMooseObject("hippoApp", FoamSideAdvectiveFluxIntegral);

InputParameters
FoamSideAdvectiveFluxIntegral::validParams()
{
  auto params = FoamSidePostprocessor::validParams();
  params.addClassDescription(
      "Class that calculates the average or scalar on a OpenFOAM boundary patch.");
  params.addRequiredParam<std::string>("foam_scalar", "Foam scalar being advected.");
  params.addParam<std::string>("advective_velocity", "U", "Advection velocity");
  return params;
}

FoamSideAdvectiveFluxIntegral::FoamSideAdvectiveFluxIntegral(const InputParameters & params)
  : FoamSidePostprocessor(params),
    _value(0.),
    _foam_scalar(params.get<std::string>("foam_scalar")),
    _advection_velocity(params.get<std::string>("advective_velocity"))
{

  if (!_foam_mesh->foundObject<Foam::volScalarField>(_foam_scalar))
    mooseError("foam_scalar '", _foam_scalar, "' not found.");

  if (!_foam_mesh->foundObject<Foam::volVectorField>(_advection_velocity))
    mooseError("advective_velocity '", _advection_velocity, "' not found.");
}

void
FoamSideAdvectiveFluxIntegral::compute()
{
  _value = 0.;
  for (auto & block : ElementUserObject::blocks())
  {
    auto & var_array =
        _foam_mesh->boundary()[block].lookupPatchField<Foam::volScalarField, double>(_foam_scalar);

    auto & vel_array = _foam_mesh->boundary()[block].lookupPatchField<Foam::volVectorField, double>(
        _advection_velocity);

    auto & areas = _foam_mesh->boundary()[block].magSf();
    auto && normals = _foam_mesh->boundary()[block].nf();

    for (int i = 0; i < var_array.size(); ++i)
    {
      _value += var_array[i] * areas[i] * (normals->data()[i] & vel_array[i]);
    }
  }

  gatherSum(_value);
}

PostprocessorValue
FoamSideAdvectiveFluxIntegral::getValue() const
{
  return _value;
}
