# FoamSideAverageValue

!syntax description /UserObjects/FoamSideAverageValue

An OpenFOAM analogue to MOOSE's `SideAverageValue` postprocessor.
It calculates the average of a `volScalarField` or `volVectorField` over an OpenFOAM boundary patch.
If a vector is specified, `component` must also be specified and must take a value of `x`, `y`, `z`, `normal` or magnitude.

## Example Input File Syntax

To calculate the average wall temperature over a boundary patch

```
[Postprocessors]
    [T_avg]
        type = FoamSideAverageValue
        foam_variable = T
        boundary = wall
    []
[]
```

!syntax parameters /UserObjects/FoamSideAverageValue

!syntax inputs /UserObjects/FoamSideAverageValue

!syntax children /UserObjects/FoamSideAverageValue
