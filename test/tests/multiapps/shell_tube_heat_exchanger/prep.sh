#!/bin/bash

set -e

usage() { echo "Usage: $0 [-e endtime] [-w writetime] [-d decompose?]" 1>&2; exit 1; }

while getopts ":e:w:d" o; do
    case "${o}" in
        e)
            ENDTIME=${OPTARG}
            ;;
        w)
            WRITETIME=${OPTARG}
            ;;
        d)
            DECOMPOSE=true
            ;;
        *)
            usage
            ;;
    esac
done

foamCleanCase -case fluid_inner
foamCleanCase -case fluid_outer

./download-meshes.sh

if [ "$DECOMPOSE" == "true" ]; then
    decomposePar -case fluid_inner
    decomposePar -case fluid_outer
fi

if [ ! -z ${ENDTIME+x} ]; then
    echo "Updating end time to $ENDTIME"

    sed -i "112s/.*/    end_time = $ENDTIME/" solid.i
    sed -i "34s/.*/endTime $ENDTIME;/" fluid_outer/system/controlDict
    sed -i "30s/.*/endTime             $ENDTIME;/" fluid_inner/system/controlDict

fi

if [ ! -z ${WRITETIME+x} ]; then
    echo "Updating write interval time to $WRITETIME"

    sed -i "30s/.*/writeInterval $WRITETIME;/" fluid_outer/system/controlDict
    sed -i "28s/.*/writeInterval       $WRITETIME;/" fluid_inner/system/controlDict

fi
