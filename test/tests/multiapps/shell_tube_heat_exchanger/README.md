# Shell and tube heat exchanger

Here we solve the preCICE [shell and tube heat exchanger](https://precice.org/tutorials-heat-exchanger.html) problem using Hippo.

![Shell and tube heat exchanger](https://precice.org/images/tutorials-heat-exchanger-visualization.png)

The fluid and solid meshes are copied from the tutorial with the solid mesh already converted to the Exodus II format for MOOSE. The case is run using FFTB (heat flux from fluid to solid, wall temperature from solid to fluid) coupling, meaning the solid is run transient. To speed up convergence, the $c_p$ is decreased initially.

`download-meshes.sh` shell script downloads and decompresses the OpenFOAM meshes, while `prep.sh` takes command line arguments to set the write interval and end time of the simulations.

There are two tests:
- **Short test:** run for 10 seconds as part of normal CI pipeline
- **Heavy test:** run for full 500 seconds and can be activated using the `--heavy` option for the `run_tests` executable.
