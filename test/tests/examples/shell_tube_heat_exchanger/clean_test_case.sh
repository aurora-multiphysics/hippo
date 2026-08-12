#!/bin/bash

set -e -u -o pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
cd "$script_dir"

rm -rf fluid-inner-openfoam fluid-outer-openfoam
rm -f README.md clean.sh download_meshes.sh *.i post.py run.sh solid.exo
rm -f solid_out.e
