#!/usr/bin/env sh

# Copy and rename this file to 'prefs.sh' into one of the following directories
# to override OpenFOAM's build options.
#  - ~/.OpenFOAM/$WM_PROJECT_VERSION
#  - ~/.OpenFOAM
#
# In particular, we use this file to provide consistent build options within
# our docker container.
# Note that this file is executed by <OpenFOAM>/etc/bashrc, so changes to this
# file will only take effect once the RC file is re-sourced.

#- Compiler location:
#    WM_COMPILER_TYPE= system | ThirdParty (OpenFOAM)
export WM_COMPILER_TYPE=system

#- Compiler:
#    WM_COMPILER = Gcc | Gcc48 ... Gcc62 | Clang | Icx
export WM_COMPILER=Gcc

#- Memory addressing:
#    On a 64bit OS this can be 32bit or 64bit
#    On a 32bit OS addressing is 32bit and this option is not used
#    WM_ARCH_OPTION = 32 | 64
export WM_ARCH_OPTION=64

#- Precision:
#    WM_PRECISION_OPTION = SP | DP | LP
export WM_PRECISION_OPTION=DP

#- Label size:
#    WM_LABEL_SIZE = 32 | 64
export WM_LABEL_SIZE=32

#- Optimised, debug, profiling:
#    WM_COMPILE_OPTION = Opt | Debug | Prof
export WM_COMPILE_OPTION=Opt

#- MPI implementation:
#    WM_MPLIB = SYSTEMOPENMPI | OPENMPI | SYSTEMMPI | MPICH | MPICH-GM | HPMPI
#               | MPI | FJMPI | QSMPI | SGIMPI | INTELMPI
export WM_MPLIB=SYSTEMMPI
