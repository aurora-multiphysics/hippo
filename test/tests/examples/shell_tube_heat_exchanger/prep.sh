#!/bin/bash

set -e -u -o pipefail

example_dir=../../../../examples/shell_tube_hx

./clean_test_case.sh
cp -r "$example_dir"/. .

./download_meshes.sh

decomposePar -case fluid-inner-openfoam
decomposePar -case fluid-outer-openfoam
