# Shell and tube heat exchanger

Here we solve the preCICE [shell and tube heat exchanger](https://precice.org/tutorials-heat-exchanger.html) problem using Hippo.

![Shell and tube heat exchanger](https://precice.org/images/tutorials-heat-exchanger-visualization.png)

The fluid and solid meshes are copied from the tutorial with the solid mesh already converted to the Exodus II format for MOOSE. The case is run using FFTB (heat flux from fluid to solid, wall temperature from solid to fluid) coupling, meaning the solid is run transient. To speed up convergence, the $c_p$ is decreased initially.

`prep.sh` cleans the case, downloads and decompresses the meshes, and partitions the case.

The regression test runs for 10 seconds, but to get a mostly converged solution, it is recommended that simulation the end time is changed to 500 by modifying `solid.i` and the `controlDict` files in `fluid_inner` and `fluid_outer`.
