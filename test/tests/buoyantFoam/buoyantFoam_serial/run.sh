#!/usr/bin/env bash

# You need OpenFOAM's bin on the system path to run this
(
    cd buoyantCavity && \
    blockMesh && \
    ./Allrun
)
$1 -i run.i
