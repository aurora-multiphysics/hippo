# FoamFixedGradientBC

!syntax description /FoamBCs/FoamFixedGradientBC

Imposes Neumann or Flux boundary conditions on OpenFOAM fields using MOOSE input file syntax. Under-the-hood, a auxilliary variable is created in the mirror mesh and the value of this variable
(whether specified through a transfer of AuxKernel)
is imposed as a fixedGradient boundary patch.
If `diffusivity` is specified, the MOOSE variable is imposed a flux,
otherwise as a simple Neumann boundary condition.

## Example Input File Syntax

In the case that `diffusivity` is not specified

```
[FoamBCs]
    [neumann]
        type = FoamFixedGradientBC
        foam_variable = T
        boundary = wall
    []
    ...
[]
```

If the diffusivity is specified and we wish to apply a heat flux boundary conditions such as in conjugate heat transfer, we use

```
[FoamBCs]
    [heat_flux]
        type = FoamFixedGradientBC
        foam_variable = T
        boundary = wall
        diffusivity = kappa
    []
    ...
[]
```

where `kappa` is the fluid thermal conductivity in OpenFOAM.


!syntax parameters /FoamBCs/FoamFixedGradientBC

!syntax inputs /FoamBCs/FoamFixedGradientBC

!syntax children /FoamBCs/FoamFixedGradientBC
