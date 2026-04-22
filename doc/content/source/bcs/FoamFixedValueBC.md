# FoamFixedValueBC

!syntax description /FoamBCs/FoamFixedValueBC

This class is used to impose Dirichlet boundary conditions on OpenFOAM boundary patches. Like all variable-based OpenFOAM boundary conditions, `FoamFixedValueBC` creates an auxilliary variable under the hood, which is copied to the boundary patch.

## Example Input File Syntax

```
[FoamBCs]
    [dirichlet]
        type = FoamFixedValueBC
        foam_variable = T
        boundary = wall
    []
[]
```

!syntax parameters /FoamBCs/FoamFixedValueBC

!syntax inputs /FoamBCs/FoamFixedValueBC

!syntax children /FoamBCs/FoamFixedValueBC
