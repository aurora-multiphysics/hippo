#! /usr/bin/env bash
## Create input files for simple mesh decomposition problem
## So we can some property based testing !!!
## Expects an input file with format (will also be generated)
##### <Number of x cells> <Number of y cells> <Number of z cells>
##### <Number of x domains> <Number of y domains> <Number of z domains>
##### <Use Wall-1> <Use Wall-2> ......
## e.g.
#10 10 12
#2 1 2
#1 1 0 0 0 0
# will have 10x10x12 cube
# partitioned in the x and z dimension
# using wall-1 and wall-2

if [ "$#" -ne 1 ]; then
    echo "Usage: ./make_test.sh <config file>"
    exit 1
fi
CONFIG=$1
#read config into variables
read -r nx ny nz < <(sed -n '1p'  $CONFIG)
read -r dx dy dz < <(sed -n '2p'  $CONFIG)
read -r w1 w2 w3 w4 w5 w6 < <(sed -n '3p' $CONFIG)
CUBE_IN=../cube_in
TESTDIR=.
#make a dir based on the input
#CUBE_OUT=${TESTDIR}/${nx}_${ny}_${nz}_${dx}_${dy}_${dz}_${w1}${w2}${w3}${w4}${w5}${w6}
CUBE_OUT=${TESTDIR}
#check we didn't already do this test
#if [ -d ${CUBE_OUT} ]; then
#    echo "${CUBE_OUT} already exists"
#    exit 0
#fi
FOAM_INPUT=${CUBE_OUT}/foaminput/system
mkdir -p ${FOAM_INPUT}

#Create blockMeshDict
sed "s/#NX/${nx}/g;s/#NY/${ny}/g;s/#NZ/${nz}/g" ${CUBE_IN}/blockMeshDict.in > ${FOAM_INPUT}/blockMeshDict
#calc number of subdomains
dn=$((${dx} * ${dy} * ${dz}))
#create decomposeParDict
sed "s/#DN/${dn}/g;s/#DX/${dx}/g;s/#DY/${dy}/g;s/#DZ/${dz}/g" ${CUBE_IN}/decomposeParDict.in > ${FOAM_INPUT}/decomposeParDict
#copy the controlDict
cp ${CUBE_IN}/controlDict ${FOAM_INPUT}

if ! command -v blockMesh $> /dev/null
then
    echo "Can't find blockMesh make sure it is in your PATH"
    exit 1
fi
if ! command -v decomposePar $> /dev/null
then
    echo "Can't find decompsePar make sure it is in your PATH"
    exit 1
fi

#Create the Moose input
awk -v var="${w1} ${w2} ${w3} ${w4} ${w5} ${w6}" \
    'BEGIN {split(var,a," ");
    for (w in a) {
        if (a[w] ~/1/) {str = str " Wall-" (count+0);}
            count++;
        }
    }
    /foam_patch/ { printf("  foam_patch = '\''%s'\''\n",str); next;}
    {print $0}' ${CUBE_IN}/run.i.in > ${CUBE_OUT}/run.i
#OpenFOAM crashes if you add -parallel to the commandline if you
#only have 1 rank for some reason.
#UNTESTED Hack to remove the parallel option from any non parallel runs
if [ ${dx} -eq 1 ] && [ ${dy} -eq 1 ] && [ ${dz} -eq 1]
then
    sed -i 's/-parallel//' ${CUBE_OUT}/run.i
fi

#Make A little script to run the problem
MPICOMMAND=mpirun
printf "%s -n %d \$1 --keep-cout -i run.i > std.out\n" $MPICOMMAND  ${dn} > ${CUBE_OUT}/run.sh
chmod +x ${CUBE_OUT}/run.sh

#Create the openfoam inputfiles
cd ${CUBE_OUT}/foaminput
blockMesh
decomposePar
