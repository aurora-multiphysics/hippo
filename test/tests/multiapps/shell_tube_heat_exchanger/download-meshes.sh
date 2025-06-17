#!/bin/sh


echo "This tutorial is based on a case prepared with SimScale."
echo "Since the mesh files are several MB large, we don't store them in the Git repository."
echo "This script downloads and extracts the missing files."
echo ""

echo "Downloading and extracting the Inner-Fluid mesh..."
wget -nv -O - https://syncandshare.lrz.de/dl/fiNsYGC1DKzgio4jS5NhsXg7/polyMesh.org.tar.gz | tar -xzv -C fluid_inner/constant
mv fluid_inner/constant/polyMesh.org fluid_inner/constant/polyMesh
gzip -d -q fluid_inner/constant/polyMesh/*


echo "Downloading and extracting the Outer-Fluid mesh..."
wget -nv -O - https://syncandshare.lrz.de/dl/fiEZRQ8rcVWRkoyZvANim1R1/polyMesh.org.tar.gz | tar -xzv -C fluid_outer/constant
mv fluid_outer/constant/polyMesh.org fluid_outer/constant/polyMesh
gzip -d -q fluid_outer/constant/polyMesh/*

echo "Update boundary type"
sed -i 's/mapped/wall/g'  fluid_inner/constant/polyMesh/boundary
sed -i 's/mapped/wall/g'  fluid_outer/constant/polyMesh/boundary

echo "Completed."
