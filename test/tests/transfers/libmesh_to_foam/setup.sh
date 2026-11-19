#!/usr/bin/env bash
set -euo pipefail

# Generate cases in the calling test directory, using the shared template.
template_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)/case_template"

parent_ranks=1
child_ranks=1
if [[ "${1:-serial}" == mpi_4_2 ]]; then
    parent_ranks=4
    child_ranks=2
fi

mesh_profile=${2:-shape}

for case_dir in parent_case child_case; do
    rm -rf -- "$case_dir"
    cp -R -- "$template_dir" "$case_dir"
    if [[ "$case_dir" == child_case && "$mesh_profile" == nearest ]]; then
        # Different face centers make nearest sampling distinguishable from interpolation.
        sed -i 's/(4 3 2) simpleGrading/(4 4 3) simpleGrading/' "$case_dir/system/blockMeshDict"
    fi
    blockMesh -case "$case_dir"

    if [[ "$case_dir" == parent_case ]]; then
        ranks=$parent_ranks
        split="$ranks 1 1"
    else
        ranks=$child_ranks
        split="1 1 $ranks"
    fi

    if (( ranks > 1 )); then
        sed -i \
            -e "s/numberOfSubdomains 1;/numberOfSubdomains $ranks;/" \
            -e "s/n (1 1 1);/n ($split);/" \
            "$case_dir/system/decomposeParDict"
        decomposePar -case "$case_dir"
    fi
done
