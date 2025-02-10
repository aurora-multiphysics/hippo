# Step-by-step Example

Setting up a coupled conjugate heat transfer problem
can broadly be boiled down to the following steps:

1. Create an OpenFOAM case.
   - Including an OpenFOAM mesh.
2. Write a MOOSE input file to run that case.
   - You'll need to specify the coupled boundaries and the time step.
3. Write a MOOSE input file to solve the MOOSE part of the problem and
   include the previous MOOSE input file as a
   [Multiapp](https://mooseframework.inl.gov/syntax/MultiApps/index.html).
      - Use the Multiapp system to set boundary conditions and couple the
    parts of the solve.

In this example we'll run through how to set up the
'Flow Over Heated Plate' example defined on
[PreCICE's website](https://precice.org/tutorials-flow-over-heated-plate.html).
We'll use OpenFOAM to solve the heat transfer problem in the fluid domain
and use MOOSE's heat conduction module to solve the solid domain.

We're going to use the 'heat flux forward' method
to solve the conjugate heat transfer problem:
we transfer the heat flux the same way as the heat is flowing.
As the solid domain is hot in this example,
we will transfer the heat flux from the MOOSE solve
and set it as a Neumann boundary condition on the OpenFOAM case.
We will transfer the wall temperature from OpenFOAM
and set that as a Dirichlet boundary condition on the MOOSE solve.
These transfers will occur at every time step.

<!-- TODO: add an image here -->

## Setting Up the OpenFOAM Case

Generally, when setting up an OpenFOAM case,
you start by looking for similar examples in your OpenFOAM installation's
`tutorials` folder and tweaking the settings.
In this case, however, PreCICE has already defined an OpenFOAM case for this
problem.
Download the `fluid-openfoam` folder from the
[`precice/tutorials` GitHub](https://github.com/precice/tutorials/tree/master/flow-over-heated-plate/fluid-openfoam).

The OpenFOAM mesh is defined in `fluid-openfoam/system/blockMeshDict`.
To generate the mesh, run `blockMesh` on the `fluid-openfoam` folder:

```shell
$ blockMesh -case fluid-openfoam
```

To decompose the mesh for parallel execution,
use OpenFOAM's `decomposePar` tool.

```shell
$ decomposePar -case fluid-openfoam
```

The case is set up to use 2 processors.
You can change this within the `fluid-openfoam/system/decomposeParDict` file.

For more info on the OpenFOAM case structure, see the
[OpenFOAM docs](https://doc.cfd.direct/openfoam/user-guide-v12/case-file-structure).

## Write a MOOSE Input File for the OpenFOAM Case

### Mesh

The first block to define is the mesh block.
Hippo provides the `FoamMesh` class to
read an OpenFOAM mesh and generate a MOOSE mesh along given boundaries.

In this example, the boundary we're interested in
coupling to the MOOSE solve is called 'interface'.

```toml
# fluid.i

[Mesh]
    type = FoamMesh
    case = 'fluid-openfoam'  # the directory of the OpenFOAM case
    foam_patch = 'interface' # the name of the coupled boundary
[]
```

You can use the `--mesh-only` option to have a look at the boundary mesh
Hippo generates for you.

```shell
$ hippo-opt -i fluid.i --mesh-only
$ paraview fluid_in.e
```

### Auxiliary Variables

To transfer quantities between the OpenFOAM and MOOSE solves,
you must use MOOSE's
[AuxVariable system](https://mooseframework.inl.gov/syntax/AuxVariables/).
For this problem we need two variables,
one for the solid heat flux and one for the fluid wall temperature.

At present, all AuxVariables used for these transfers must
be constant and monomial.

```toml
# fluid.i

[AuxVariables]
    [fluid_wall_temp]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 300
    []
    [solid_heat_flux]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0
    []
[]
```

> _**Note**: To save on memory, you could actually define a single AuxVariable
> and transfer the heat flux one way and temperature the other in the same
> variable. However, for clarity, we use separate variables here._

### Problem

Next we define the 'Problem' block.
Hippo provides the `FoamProblem` class to run the OpenFOAM case
provided in the `[Mesh]` block.

`FoamProblem` has a set of parameters that defines which quantities are
copied to and from OpenFOAM:

- `foam_heat_flux` -
  the name of the AuxVariable to copy OpenFOAM's heat flux into.
- `foam_temp` -
  the name of the AuxVariable to copy OpenFOAM's wall temperature into.
- `temp` -
  the name of an AuxVariable whose values will be used to set
  a Dirichlet boundary condition on the OpenFOAM solve.
- `heat_flux`
  the name of an AuxVariable whose values will be used to set
  a Neumann boundary condition on the OpenFOAM solve.

In our example, we'll need to set `heat_flux` and `foam_temp`.

```toml
# fluid.i

[Problem]
    type = FoamProblem
    # Take the heat flux from MOOSE and set it on the OpenFOAM mesh.
    heat_flux = solid_heat_flux
    # Take the boundary temperature from OpenFOAM and set it on the MOOSE mesh.
    foam_temp = fluid_wall_temp
[]
```

## Executioner

Set the time step settings in the `Executioner` block.
Hippo provides the `FoamTimeStepper` class to set the
time step configuration of the OpenFOAM solve.
These settings will override any time step settings
in the OpenFOAM case's `controlDict`.

```toml
# fluid.i

[Executioner]
    type = Transient
    start_time = 0
    end_time = 1
    dt = 0.025

    [TimeStepper]
        type = FoamTimeStepper
    []
[]
```

You can now run the OpenFOAM case using `hippo-opt -i fluid.i`.
Although the result won't be very interesting as
we haven't set up the solid domain of the problem yet.
