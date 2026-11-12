#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"

case "${1:-serial}" in
    serial) parent_ranks=1; child_ranks=1 ;;
    mpi_2_2) parent_ranks=2; child_ranks=2 ;;
    mpi_4_2) parent_ranks=4; child_ranks=2 ;;
    *) echo "Usage: $0 [serial|mpi_2_2|mpi_4_2]" >&2; exit 1 ;;
esac

for case_dir in parent_case child_case; do
    rm -rf -- "$case_dir"
    cp -R -- case_template "$case_dir"
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
