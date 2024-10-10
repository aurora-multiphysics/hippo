# Simple MultiApp (Parallel)

This is a simple multiapp that runs for one time step,
that checks the initial temperature from a MOOSE mesh's boundary
is successfully transferred to the OpenFOAM boundary.

To regenerate the OpenFOAM mesh:

```console
.../hippo-opt -i fluid_mesh.i --mesh-only fluid_mesh.msh
cd buoyantCavity
gmshToFoam ../fluid_mesh.msh
decomposePar
```
