#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"
foamCleanCase -case foam/
blockMesh -case foam
# Non-unit end areas distinguish heat-flux averages from integrals.
transformPoints -case foam 'scale=(1 1 2)'
decomposePar -case foam -force
