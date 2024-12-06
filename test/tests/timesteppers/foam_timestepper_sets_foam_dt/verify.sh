#!/usr/bin/env bash
#
# Verify that the OpenFOAM time directories are the ones we'd expect given the
# '[Executioner]' block in the MOOSE input file.

set -e

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
CASE_DIR="${SCRIPT_DIR}/buoyantCavity"

for t in $(seq 1.5 0.5 2.5); do
    t="${t//.0/}"
    if ! [ -d "${CASE_DIR}/$t" ]; then
        echo "Error for t=$t"
        exit 1
    fi
done

if [ -d "${CASE_DIR}/0.5" ]; then
    echo "Time step 0.5 exists when it shouldn't"
    exit 1
fi
if [ -d "${CASE_DIR}/3" ]; then
    echo "Time step 3 exists when it shouldn't"
    exit 1
fi
