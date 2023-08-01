#! /usr/bin/env bash

#Check output of generated test case
#Currently just looks at number of elements
#i.e. calculates the expected number of elements
#from the config and then uses exodiff --summary to check
#it's the same


if [ "$#" -ne 2 ]; then
    echo "Usage: ./check_out.sh <config file> <moose output>"
    exit 1
fi
EXODIFF=${MOOSE_DIR}/framework/contrib/exodiff/exodiff
if ! command -v $EXODIFF $> /dev/null
then
    echo "Can't find exodiff"
    exit 1
fi
#Obscurely stupid bit of awk but the openfoam blockMeshDict 
#is set up such that 
#0 and 3 faces are normal to the y,
#1 and 4 faces are normal to the x,
#2 and 5 faces are normal to the z,
#We just count the number of elements on each face that is "on"
#i.e. 1 in the input
expected_elem=$(awk 'NR==1 {nx=$1; ny=$2; nz=$3}
                     NR==3 {for (i=0; i<NF; i++) {
                     if ($(i+1)~/1/) {
                       face = i%3
                       if (face==0) {
                           sum += nx*nz;
                       } else if (face==1) {
                          sum += nz*ny;
                       } else {
                          sum += nx*ny;
                    }}} 
                    print sum;}' $1)
#Get the same number from exodiff
actual_elem=$( $EXODIFF --summary $3 | awk '/Elements/ {print $13 + 0}')

if [ $actual_elem -ne $expected_elem ]; then
    echo "unexpected number of elements in moose mesh == $actual_elem, expecting $expected_elem"
    exit 1
fi
echo "Passed"
exit 0


