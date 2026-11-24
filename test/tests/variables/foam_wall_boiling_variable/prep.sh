#!/usr/bin/env bash
# Clean generated results, preserving the checked-in initial conditions.
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"

foamCleanCase -case foam
blockMesh -case foam
extrudeMesh -case foam
decomposePar -case foam -force
rm -f -- main_out.e*
