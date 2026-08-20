# Shell and tube heat exchanger

Here we solve the preCICE [shell and tube heat exchanger](https://precice.org/tutorials-heat-exchanger.html)
problem using Hippo.

![Shell and tube heat exchanger](https://precice.org/images/tutorials-heat-exchanger-visualization.png)

The inner and outer fluid cases have been adapted from the preCICE tutorial
found [here](https://github.com/precice/tutorials/tree/develop/heat-exchanger).
The mesh files are downloaded by `download_meshes.sh` and are distributed under
the LGPL v3.0 license. The solid mesh has already been converted to the Exodus II
format for MOOSE. The case uses FFTB coupling (heat flux from fluid to solid and
wall temperature from solid to fluid), so the solid runs transiently. To speed up
convergence, its $c_p$ is decreased initially.

Run the example with `./run.sh`. The script cleans the case, downloads and
decompresses the meshes, partitions the fluid domains, and launches Hippo.

The input files run to 500 seconds to obtain a mostly converged solution. The
automated smoke test copies this example and overrides the end time to one
timestep.
