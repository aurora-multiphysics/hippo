#!/usr/bin/env bash

# You need OpenFOAM's bin on the system path to run this
(
    cd buoyantCavity && \
    blockMesh && \
    decomposePar && \
    ./Allrun
)
mpirun -n 4 $1 -i run.i
