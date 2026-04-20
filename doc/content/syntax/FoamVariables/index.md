# FoamVariables System

## Overview

The `FoamVariable`s system controls how OpenFOAM scalar variables are shadowed by
counterpart MOOSE variables,
allowing the OpenFOAM quantities to be used as boundary conditions in libMesh-based Moose apps.

There are two types of `FoamVariable`s
- `FoamVariableField`: for shadowing OpenFOAM `volScalarField`s
- `FoamFunctionObject`: for shadowing the scalar outputs of OpenFOAM `functionObjects`

Under the hood, the FoamVariables create a constant monomial MooseVariable,
with data transferred from the OpenFOAM variable to a MOOSE variable within the Hippo sub-app
after the OpenFOAM step is complete. This can then be transferred to other MOOSE apps like any other
MOOSE variable.
A more thorough explanation of Hippo's 'mirror mesh' and data transfer can be found here.

## Example

```
[FoamVariables]
    [temperature]
        type = FoamVariableField
        foam_variable = T
    []
    [heat_flux]
        type = FoamFunctionObject
        foam_variable = wallHeatFlux
    []
[]
```

!syntax list /FoamVariables objects=True actions=False subsystems=False

!syntax list /FoamVariables objects=False actions=False subsystems=True

!syntax list /FoamVariables objects=False actions=True subsystems=False
