# FoamFunctionObject

!syntax description /FoamVariables/FoamFunctionObject

## Overview

Shadows the output of OpenFOAM function objects.
Currently restricted to the `wallHeatFlux` function object,
which is executed at the end of each OpenFOAM time step.
In all other ways it behaves the same as `FoamVariableField`.

## Example Input File Syntax

```
[FoamVariables]
    [heat_flux]
        type = FoamFunctionObject
        foam_variable = wallHeatFlux
    []
[]
```

!syntax parameters /FoamVariables/FoamFunctionObject

!syntax inputs /FoamVariables/FoamFunctionObject

!syntax children /FoamVariables/FoamFunctionObject
