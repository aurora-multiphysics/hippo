#include "HeatFluxAux.h"

#include <Assembly.h>
#include <MooseError.h>

registerMooseObject("hippoApp", HeatFluxAux);

namespace
{
inline Real
dot_product(const int n_dimensions, const Point & X, const Point & Y)
{
  Real tot = 0.0;
  for (int i = 0; i < n_dimensions; ++i)
  {
    tot += X(i) * Y(i);
  }
  return tot;
}
}

InputParameters
HeatFluxAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("T", "Temperature field");
  params.addParam<MaterialPropertyName>("diffusion_coefficient",
                                        "thermal_conductivity",
                                        "Property name of the diffusion coefficient");
  params.addParam<Real>("scale", 1.0, "Scaling factor for heat flux");
  // Heat flux can only be calculated on a boundary.
  params.makeParamRequired<std::vector<BoundaryName>>("boundary");
  return params;
}

HeatFluxAux::HeatFluxAux(const InputParameters & params)
  : AuxKernel(params),
    _grad{coupledGradient("T")},
    _thermal_conductivity{getMaterialProperty<Real>("diffusion_coefficient")},
    _normals{_assembly.normals()},
    _dim{_mesh.dimension()},
    _scale{getParam<Real>("scale")}
{
  mooseAssert(0 < _dim && _dim <= 3, "Dimension should be 1, 2 or 3");
}

Real
HeatFluxAux::computeValue()
{
  mooseAssert(_qp < _grad.size(), "Cannot index _grad with _qp");
  mooseAssert(_qp < _normals.size(), "Cannot index _normals with _qp");
  return -dot_product(_dim, _grad[_qp], _normals[_qp]) * _thermal_conductivity[_qp] * _scale;
}
