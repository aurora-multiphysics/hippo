#!/bin/bash

set -e -u -o pipefail

foamCleanCase -case fluid_inner
foamCleanCase -case fluid_outer

echo "Extracting the Inner-Fluid mesh..."

tar -xvf fluid_inner_mesh.tar.gz
mv polyMesh.org fluid_inner/constant/polyMesh
gzip -d -q fluid_inner/constant/polyMesh/*


echo "Extracting the Outer-Fluid mesh..."
# repeat for outer mesh
tar -xvf fluid_outer_mesh.tar.gz
mv polyMesh.org fluid_outer/constant/polyMesh
gzip -d -q fluid_outer/constant/polyMesh/*

#Update boundary types as original mesh is for preCICE tutorial
echo "Update boundary type"
sed -i 's/mapped/wall/g'  fluid_inner/constant/polyMesh/boundary
sed -i 's/mapped/wall/g'  fluid_outer/constant/polyMesh/boundary

#partition mesh
decomposePar -case fluid_inner
decomposePar -case fluid_outer
