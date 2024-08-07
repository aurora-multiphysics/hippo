#!/usr/bin/env sh
set -e -u

. ../../tools/cleaning-tools.sh

clean_openfoam .

set -e -u

(
    cd "$1"
    echo "- Cleaning up OpenFOAM case in $(pwd)"
    if [ -n "${WM_PROJECT:-}" ] || error "No OpenFOAM environment is active."; then
        # shellcheck disable=SC1090 # This is an OpenFOAM file which we don't need to check
        . "${WM_PROJECT_DIR}/bin/tools/CleanFunctions"
        cleanCase > /dev/null
        rm -rfv 0/uniform/functionObjects/functionObjectProperties history
    fi
)
