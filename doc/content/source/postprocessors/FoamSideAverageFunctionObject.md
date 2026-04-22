# FoamSideAverageFunctionObject

!syntax description /UserObjects/FoamSideAverageFunctionObject


This is the same as `FoamSideAverageValue` but for function objects rather than variables.
Calculates the average of a `volScalarField` or `volVectorField` function object over an OpenFOAM boundary patch.
If a vector is specified, a component must also be given.

## Example Input File Syntax


To calculate the average heat flux through a surface, we can average the `wallHeatFlux` function object over a boundary patch.

```
[Postprocessors]
    [q_avg]
        type = FoamSideAverageFunctionObject
        function_object = wallHeatFlux
        boundary=wall
    []
[]
```

!syntax parameters /UserObjects/FoamSideAverageFunctionObject

!syntax inputs /UserObjects/FoamSideAverageFunctionObject

!syntax children /UserObjects/FoamSideAverageFunctionObject
