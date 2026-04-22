# FoamSideIntegratedFunctionObject


!syntax description /UserObjects/FoamSideIntegratedFunctionObject

This is the same as `FoamSideIntegratedValue` but for function objects rather than variables.
If a vector is specified, a component must also be given.

## Example Input File Syntax

To calculate the total heat transferred through a surface, we can integrate the `wallHeatFlux` function object over a boundary patch.

```
[Postprocessors]
    [q]
        type = FoamSideIntegratedFunctionObject
        function_object = wallHeatFlux
        boundary=wall
    []
[]
```

!syntax parameters /UserObjects/FoamSideIntegratedFunctionObject

!syntax inputs /UserObjects/FoamSideIntegratedFunctionObject

!syntax children /UserObjects/FoamSideIntegratedFunctionObject
