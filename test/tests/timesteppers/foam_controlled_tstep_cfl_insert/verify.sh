#!/usr/bin/env bash
#
# Verify that the OpenFOAM time directories are the ones we'd expect given the
# '[Executioner]' block in the MOOSE input file.

set -e

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
CASE_DIR="${SCRIPT_DIR}/fluid-openfoam"

for t in 0.1 0.2 0.3 0.4 0.5; do
    if ! [ -d "${CASE_DIR}/$t" ]; then
        echo "Error for t=$t"
        exit 1
    fi
done
