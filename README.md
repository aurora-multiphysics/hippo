hippo
=====

Moose app that wraps OpenFOAM buoyantFoam solver.

# Install/Build

NOTE: Only tested with mpich and gcc

# OpenFOAM
Unfortunatly hippo relies on a patching openFOAM, there is a script `scripts/get_openfoam.sh` that will clone OpenFOAM and apply the patch e.g.

```
cd scripts/
./get_openfoam.sh <path/to/where/you/want/openFOAM>
```

Once cloned build following instructions [Here](https://openfoam.org/download/10-source/) to build from source

# Moose

Follow instructions [Here](https://mooseframework.inl.gov/getting_started/installation/gcc_install_moose.html) to build MOOSE

# Hippo

### Environment

Set (recommend [direnv](https://direnv.net/) for this) 

```
MOOSE_DIR=/path/to/moose/dir
MPI_ROOT=/path/to/mpich/root
MPI_ARCH_INC="-I/path/to/mpi/includes"
MPI_ARCH_LIBS="-L/path/to/mpi/libs -lmpi"
source /path/to/OpenFOAM/etc/bashrc
```

Once everything else is built then (in theory) should be able to `make` in the root directory (or `METHOD=dbg make` for a debug build

### Tests
Some basic tests can be run with `./run_tests`


# NOTES

This work in progress so far hippo can:
 - Create a moose mesh from the boundary of an openfoam mesh
 - Can run an buoyantFOAM case via moose
 - Transfer temperature from faces of openfoam mesh to nodes of Moose mesh

NOTE: When running in parallel add `--keep-cout` to the command line because OpenFOAM will crash if
a process fails to write to stdout out

"Fork hippo" to create a new MOOSE-based application.

For more information see: [http://mooseframework.org/create-an-app/](http://mooseframework.org/create-an-app/)
