#!/usr/bin/env bash

if [ "$#" -ne 1 ]; then
    echo "Usage: ./get_openfoam.sh <path/to/put/openfoam>"
    exit 1
fi
git clone  https://github.com/OpenFOAM/OpenFOAM-10.git $1
cp openfoam.patch $1/
cd $1
git checkout d72c3ccf156ba0191a0a090e651a0da0a96a83a3
git apply openfoam.patch
