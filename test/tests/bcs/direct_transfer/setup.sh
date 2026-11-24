#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
template_dir="$script_dir/case_template"

# Assemble the thermal case before decomposition so processor fields inherit it.
rm -rf -- child_case
cp -R -- "$template_dir" child_case
blockMesh -case child_case

# mpi_4_2 runs four parent ranks and two child ranks, only the child needs OpenFOAM decomposition.
if [[ "${1:-serial}" == mpi_4_2 ]]; then
    sed -i \
        -e 's/numberOfSubdomains 1;/numberOfSubdomains 2;/' \
        -e 's/n (1 1 1);/n (1 1 2);/' child_case/system/decomposeParDict
    decomposePar -case child_case
fi
