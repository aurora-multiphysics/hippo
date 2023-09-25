#!/usr/bin/env sh

# Copy and rename this file to 'prefs.sh' into one of the following directories
# to override OpenFOAM's build options.
#  - ~/.OpenFOAM/$WM_PROJECT_VERSION
#  - ~/.OpenFOAM
#  - $FOAM_INST_DIR/site/$WM_PROJECT_VERSION
#  - $FOAM_INST_DIR/site

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
export WM_LABEL_SIZE=64

#- Optimised, debug, profiling:
#    WM_COMPILE_OPTION = Opt | Debug | Prof
export WM_COMPILE_OPTION=Opt

#- MPI implementation:
#    WM_MPLIB = SYSTEMOPENMPI | OPENMPI | SYSTEMMPI | MPICH | MPICH-GM | HPMPI
#               | MPI | FJMPI | QSMPI | SGIMPI | INTELMPI
export WM_MPLIB=SYSTEMMPI
