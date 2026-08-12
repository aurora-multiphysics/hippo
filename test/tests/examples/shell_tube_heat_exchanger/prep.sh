#!/bin/bash

set -e -u -o pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
cd "$script_dir"

example_dir=../../../../examples/shell_tube_hx

./clean_test_case.sh
cp -r "$example_dir"/. .

tar -xzf fluid_inner_mesh.tar.gz -C fluid-inner-openfoam/constant
mv fluid-inner-openfoam/constant/polyMesh.org \
    fluid-inner-openfoam/constant/polyMesh

tar -xzf fluid_outer_mesh.tar.gz -C fluid-outer-openfoam/constant
mv fluid-outer-openfoam/constant/polyMesh.org \
    fluid-outer-openfoam/constant/polyMesh

gzip -d -q fluid-inner-openfoam/constant/polyMesh/*
gzip -d -q fluid-outer-openfoam/constant/polyMesh/*

sed -i 's/mapped/wall/g' fluid-inner-openfoam/constant/polyMesh/boundary
sed -i 's/mapped/wall/g' fluid-outer-openfoam/constant/polyMesh/boundary

decomposePar -case fluid-inner-openfoam
decomposePar -case fluid-outer-openfoam
