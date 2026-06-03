#!/bin/bash
set -e

# Output file with list of times from foamRun simulation

cd fluid-openfoam
./Allclean >> /dev/null
blockMesh >> /dev/null
foamRun -solver fluid >> /dev/null
find . -name '[0-9]*' -printf "%P\n" > ../ref_times.txt
