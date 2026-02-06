#include "FoamSideIntegratedBase.h"
#include "MooseEnum.h"
#include "MooseTypes.h"
#include <volFieldsFwd.H>

InputParameters
FoamSideIntegratedBase::validParams()
{
  MooseEnum components("x y z normal magnitude", "magnitude");
  auto params = FoamSidePostprocessor::validParams();
  params.addParam<MooseEnum>(
      "component", components, "If foam variable is a vector, which component to output");
  return params;
}

FoamSideIntegratedBase::FoamSideIntegratedBase(const InputParameters & params)
  : FoamSidePostprocessor(params), _value(0.)
{
}

PostprocessorValue
FoamSideIntegratedBase::getValue() const
{
  return _value;
}

Real
FoamSideIntegratedBase::integrateValue(const std::string & variable)
{

  Real value = 0.;
  // loop over boundary ids
  for (auto & boundary : _boundary)
  {
    auto & areas = _foam_mesh->boundary()[boundary].magSf();
    Foam::Field<double> var_array;

    if (_foam_mesh->foundObject<Foam::volVectorField>(variable))
    {
      // get vector data associated with the boundary
      auto & vec_data =
          _foam_mesh->boundary()[boundary].lookupPatchField<Foam::volVectorField, double>(variable);

      // get the component specified in parameters and get the
      // component of the vector in that direction
      auto components = parameters().get<MooseEnum>("component");
      if (components == "normal")
      {
        auto && normals = _foam_mesh->boundary()[boundary].nf();
        var_array = normals & vec_data;
      }
      else if (components == "magnitude")
        var_array = Foam::mag(vec_data);
      else
        var_array = vec_data.component(int(components));
    }
    else
    {
      var_array =
          _foam_mesh->boundary()[boundary].lookupPatchField<Foam::volScalarField, double>(variable);
    }

    // Integrate
    for (int i = 0; i < var_array.size(); ++i)
    {
      value += var_array[i] * areas[i];
    }
  }

  // sum over ranks
  gatherSum(value);

  return value;
}

Real
FoamSideIntegratedBase::getArea()
{
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
  return area;
}
