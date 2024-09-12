#!/usr/bin/env bash

mpirun -n 4 $1 -i run.i
(
    cd buoyantCavity && \
    reconstructPar
)
