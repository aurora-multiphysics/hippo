#!/bin/bash

set -e

./clean.sh

./download_meshes.sh

decomposePar -case fluid-inner-openfoam
decomposePar -case fluid-outer-openfoam

mpirun -n 4 ../../hippo-opt -i solid.i
