# FoamSideIntegratedValue

!syntax description /UserObjects/FoamSideIntegratedValue

An OpenFOAM analogue to MOOSE's `SideIntegratedValue` postprocessor.
It calculates the integral of a `volScalarField` or `volVectorField` over an OpenFOAM boundary patch.
If a vector is specified, `component` must also be specified and must take a value of `x`, `y`, `z`, `normal` or magnitude.

## Example Input File Syntax

To calculate the volume flow rate over the inlet patch

```
[Postprocessors]
    [Q]
        type = FoamSideIntegratedValue
        foam_variable = U
        boundary = inlet
        component=normal
    []
[]
```


!syntax parameters /UserObjects/FoamSideIntegratedValue

!syntax inputs /UserObjects/FoamSideIntegratedValue

!syntax children /UserObjects/FoamSideIntegratedValue
