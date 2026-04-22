# FoamMesh

!syntax description /Mesh/FoamMesh

`FoamMesh` controls the interaction of the OpenFOAM mesh with MOOSE.
From a user perspective, this involves specifying the directory of the OpenFOAM cases
and the patches that are to be mirrored.
Internally, this class also creates a libMesh representation of the OpenFOAM boundary patches.


## Example Input File Syntax

Below is a simple example of `FoamMesh` for an OpenFOAM case in directory `fluid-openfoam` with a boundary patch to be coupled with MOOSE called `wall`.

```
[Mesh]
    [foam_mesh]
        type = FoamMesh
        foam_patch = wall
        case = fluid-openfoam
    []
[]
```

!syntax parameters /Mesh/FoamMesh

!syntax inputs /Mesh/FoamMesh

!syntax children /Mesh/FoamMesh
