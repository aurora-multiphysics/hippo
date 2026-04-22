# FoamProblem

!syntax description /Problem/FoamProblem

A MOOSE `ExternalProblem` that coordinates the OpenFOAM simulation from MOOSE
including the transfer of variables between the mirror mesh and OpenFOAM,
as well as the calculation of any OpenFOAM-based postprocessors.
This can be thought of as the root class for the execution of OpenFOAM simulations
within MOOSE.

```
[Problem]
    type = FoamProblem
[]
```

!syntax parameters /Problem/FoamProblem

!syntax inputs /Problem/FoamProblem

!syntax children /Problem/FoamProblem
