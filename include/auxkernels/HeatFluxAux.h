#pragma once

#include <AuxKernel.h>
#include <MooseTypes.h>

/**
 * Auxiliary kernel to calculate the heat flux at the given boundaries.
 */
class HeatFluxAux : public AuxKernel
{
public:
  HeatFluxAux(const InputParameters & params);

  static InputParameters validParams();

  /**
   * Compute the heat flux at the given quadrature point.
   *
   * Computes q = -s * k * ∇T, where 's' is a scale factor, k is the thermal conductivity, and T
   * is temperature.
   *
   * By default, the thermal_conductivity material property is used for k.
   * It can also be  specified using the diffusion_coefficient parameter.
   */
  Real computeValue();

private:
  VariableGradient const & _grad;
  const MaterialProperty<Real> & _thermal_conductivity;
  MooseArray<Point> const & _normals;
  unsigned const _dim{0};
  Real const _scale{1.0};
};
