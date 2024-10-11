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
   * Note that we are assuming the thermal conductivity is equal at every quadrature point on the
   * given boundary.
   */
  Real computeValue();

private:
  VariableGradient const & _grad;
  Real const _thermal_conductivity{0.0};
  MooseArray<Point> const & _normals;
  unsigned const _dim{0};
  Real const _scale{1.0};
};
