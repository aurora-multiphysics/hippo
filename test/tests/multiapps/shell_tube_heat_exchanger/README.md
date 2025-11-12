# Shell and tube heat exchanger

Here we solve the preCICE [shell and tube heat exchanger](https://precice.org/tutorials-heat-exchanger.html)
problem using Hippo.

![Shell and tube heat exchanger](https://precice.org/images/tutorials-heat-exchanger-visualization.png)

The inner and outer fluid cases have been adapted from the preCICE tutorial
found [here](https://github.com/precice/tutorials/tree/develop/heat-exchanger).
In particular, the mesh tar files, `fluid_inner_mesh.tar.gz` and `fluid_outer_mesh.tar.gz`,
have been downloaded using the `download-meshes.sh` script in that repository,
which is distributed under the LGPL v3.0 license and redistributed here under the
GPL v3.0 license. The solid mesh has already been converted to the Exodus II format
for MOOSE. The case is run using FFTB (heat flux from fluid to solid, wall temperature
 from solid to fluid) coupling, meaning the solid is run transient.
 To speed up convergence, the $c_p$ is decreased initially.

`prep.sh` cleans the case, downloads and decompresses the meshes, and partitions the case.

The regression test runs for 10 seconds, but to get a mostly converged solution,
it is recommended that simulation the end time is changed to 500 by modifying
`solid.i` and the `controlDict` files in `fluid_inner` and `fluid_outer`.
