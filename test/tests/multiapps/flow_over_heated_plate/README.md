# Heated Plate MultiApp

Here we solve the problem of fluid flowing over the top of a plate
which is heated from the bottom.
The example is taken from this
[preCICE](http://precice.org/tutorials-flow-over-heated-plate.html) tutorial.

![Problem diagram](./doc/heated_plate.drawio.svg)

The OpenFOAM mesh was copied from the preCICE example's
[source code](https://github.com/precice/tutorials/tree/7918d9f/flow-over-heated-plate/fluid-openfoam).

## Fluid Properties

- Thermal conductivity,    W = 100  W/(m.K)
- Viscosity,               μ = 0.0002 kg/(m.s)
- Specific Heat Capacity, Cp = 5000 J/(kg.K)
- Velocity,                U = 0.1x̂ m/s

## Solid Properties

- Thermal conductivity,    W = 100  W/(m.K)
- Thermal diffusivity,     α = 1 m2/s
- From α = k/(ρ.Cp):
  - Cp := 0.5 J/(kg.K)
  - ρ  := 100/0.5 = 200 kg/m3
