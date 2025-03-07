# Simplified Heat Exchanger

This example is taken from
[preCICE](https://precice.org/tutorials-heat-exchanger-simplified.html).

To validate the simulation, we take the temperature across the two interfaces
and compare them to the temperatures found by preCICE.
To regenerate the reference data in `gold/`,
you'll need to run the preCICE example and use the `generate_gold.py` script,
which writes the interface temperatures to two CSV files.

To help with running the preCICE case,
a Dockerfile is provided: [`precice.Dockerfile`](./precice.Dockerfile).
Mount the [preCICE tutorials](www.github.com/precice/tutorials.git)
into the container and follow the instructions within to
run the example.

Tip: use `docker exec -it <container_sha> bash` to
launch a second shell in the same container.
