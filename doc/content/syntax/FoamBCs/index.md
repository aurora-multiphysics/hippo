# FoamBCs System

## Overview

The `FoamBC` system control how boundary conditions are imposed on OpenFOAM simulations using MOOSE input file syntax.
Currently, the `FoamBC` system focuses on imposing MOOSE variable fields onto OpenFOAM boundary conditions.
The different BCs are imposed in OpenFOAM as either `fixedValue` or `fixedGradient` types reflecting Dirichlet and Neumann BCs respectively.
Currently, the correct BC type needs to be specified in OpenFOAM's `0` directory, although the details will be overriden by Hippo.

## Example Input File Syntax

```
[FoamBCs]
    [wall_temp]
        type = FoamFixedValueBC
        foam_variable = T
        boundary = wall1
    []
    [wall_heat_flux]
        type = FoamFixedGradientBC
        foam_variable = T
        boundary = wall2
        diffusivity = kappa # thermal conductivity
    []
[]
```

!syntax list /FoamBCs objects=True actions=False subsystems=False

!syntax list /FoamBCs objects=False actions=False subsystems=True

!syntax list /FoamBCs objects=False actions=True subsystems=False
