#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"

for case_dir in parent_case child_case; do
    rm -rf -- "$case_dir"
    cp -R -- case_template "$case_dir"
    blockMesh -case "$case_dir"
done
