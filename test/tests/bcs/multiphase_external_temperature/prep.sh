#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"

# Coupled and spatial reuse foam; the stock reference is retained for comparison.
case "${1:-}" in
    reference|nonphase) case_dir="$1" ;;
    coupled)
        rm -rf -- foam
        mkdir foam
        cp -r reference/{0,constant,system} foam/
        exit 0
        ;;
    spatial)
        foamListTimes -case foam -noZero -rm
        rm -rf -- foam/0
        cp -r reference/0 foam/0
        foamDictionary foam/system/controlDict -entry endTime -set 0.0002
        foamDictionary foam/system/controlDict -entry writeInterval -set 1
        # Thermo initially creates gradientEnergy and fixedEnergy, respectively.
        foamDictionary foam/0/T.liquid -entry boundaryField/wall \
            -set '{ type zeroGradient; value $internalField; }'
        foamDictionary foam/0/T.gas -entry boundaryField/wall \
            -set '{ type fixedValue; value $internalField; }'
        exit 0
        ;;
    *) echo "Usage: bash prep.sh {reference|coupled|spatial|nonphase}" >&2; exit 1 ;;
esac
rm -rf -- "$case_dir"

if [[ "$1" == nonphase ]]; then
    mkdir -p "$case_dir"/{0,constant,system}
    for file in 0/T constant/physicalProperties system/{blockMeshDict,controlDict,fvSchemes,fvSolution}; do
        cp "../fixed_value_pp/foam/$file" "$case_dir/$file"
    done
    blockMesh -case "$case_dir"
    exit 0
fi

cp -r template "$case_dir"

blockMesh -case "$case_dir"
extrudeMesh -case "$case_dir"
