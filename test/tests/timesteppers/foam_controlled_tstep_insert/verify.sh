#!/usr/bin/env bash
#
# Verify that the OpenFOAM time directories are the ones we'd expect given the
# '[Executioner]' block in the MOOSE input file.

set -e

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
CASE_DIR="${SCRIPT_DIR}/buoyantCavity"

for t in 0.4 0.8 1 1.4 1.8 2 2.4 2.8 3; do
    if ! [ -d "${CASE_DIR}/$t" ]; then
        echo "Error for t=$t"
        exit 1
    fi
done
