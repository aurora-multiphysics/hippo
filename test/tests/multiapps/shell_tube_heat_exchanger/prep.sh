#!/bin/bash

set -e -u -o pipefail

# Downloads mesh file and check the hash against known reference
downloadMesh()
{
    DIRECTORY=$1
    URL=$2
    HASH=$3
    FILE=${DIRECTORY}_mesh.tar.gz

    if [ ! -f $FILE ]; then
        wget -O $FILE -nv ${URL}
    fi

    if [ $? -ne 0 ]; then
        echo "Download failed"
        exit 1
    fi
    echo "$HASH $FILE" | sha256sum -c --status
    if [ $? -ne 0 ]; then
        echo "checksum $HASH failed"
        exit 1
    fi

    tar -xvf $FILE
    mv polyMesh.org $DIRECTORY/constant/polyMesh
    gzip -d -q $DIRECTORY/constant/polyMesh/*
}

foamCleanCase -case fluid_inner
foamCleanCase -case fluid_outer

echo "Downloading and extracting the Inner-Fluid mesh..."

#download file and check hash of files before extracting and decompressing them
inner_mesh_url=https://syncandshare.lrz.de/dl/fiNsYGC1DKzgio4jS5NhsXg7/polyMesh.org.tar.gz
inner_mesh_hash="cb367fb24caf5de07da5610fd01e492995ef040c6f684ed1b5e3f6139dd5a39c"
downloadMesh fluid_inner $inner_mesh_url "$inner_mesh_hash"


echo "Downloading and extracting the Outer-Fluid mesh..."
# repeat for outer mesh
outer_mesh_url=https://syncandshare.lrz.de/dl/fiEZRQ8rcVWRkoyZvANim1R1/polyMesh.org.tar.gz
outer_mesh_hash="aed51e0f4e198fed716694dc3e74231fdd6a9a10fbac530fb3ff4ac41895cc12"
downloadMesh fluid_outer $outer_mesh_url "$outer_mesh_hash"

#Update boundary types as original mesh is for preCICE tutorial
echo "Update boundary type"
sed -i 's/mapped/wall/g'  fluid_inner/constant/polyMesh/boundary
sed -i 's/mapped/wall/g'  fluid_outer/constant/polyMesh/boundary

#partition mesh
decomposePar -case fluid_inner
decomposePar -case fluid_outer
