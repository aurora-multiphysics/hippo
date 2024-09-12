# Simple MultiApp (Serial)

To regenerate the OpenFOAM mesh:

```console
.../hippo-opt -i fluid_mesh.i --mesh-only fluid_mesh.msh
cd buoyantCavity
gmshToFoam ../fluid_mesh.msh
decomposePar
```
