#!/bin/bash

set -e

foamCleanCase -case fluid-inner-openfoam
foamCleanCase -case fluid-outer-openfoam

rm -rf solid_out*
