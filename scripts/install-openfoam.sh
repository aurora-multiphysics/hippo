#!/usr/bin/env bash

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
OUT_DIR="$(dirname ${SCRIPT_DIR})/external/openfoam"
USAGE="usage: install-openfoam.sh [-h] [-o DIRECTORY]

Install OpenFOAM for Ubuntu, applying the patch required by hippo.

options:
  -o <DIRECTORY>  the directory to install OpenFOAM within
                  [default: '${OUT_DIR}']
  -h              show help and exit
"

REQUIREMENTS=("getopts" "tar" "wget")
for REQ in "${REQUIREMENTS[@]}"; do
    if ! command -v "${REQ}" > /dev/null; then
        >&2 echo "install-openfoam: error: ${REQ} is not installed"
        exit 1
    fi
done

while getopts "o:h" opt; do
    case "${opt}" in
        o) OUT_DIR="${OPTARG}"
            ;;
        h) echo "${USAGE}" && exit 0
            ;;
        *)  exit 1
    esac
done

OPENFOAM_DIR="${OUT_DIR}/OpenFOAM-10"
OPENFOAM_REV="d72c3ccf156ba0191a0a090e651a0da0a96a83a3"
THIRDPARTY_DIR="${OUT_DIR}/ThirdParty-10"

# Fetch and patch OpenFOAM
mkdir -p "${OPENFOAM_DIR}"
if [ ! -d "${OPENFOAM_DIR}/.git" ]; then
    git clone  https://github.com/OpenFOAM/OpenFOAM-10.git "${OPENFOAM_DIR}"
fi
git -C "${OPENFOAM_DIR}" reset --hard "${OPENFOAM_REV}"
git -C "${OPENFOAM_DIR}" apply "${SCRIPT_DIR}/openfoam.patch"

# Set up OpenFOAM
source "${OPENFOAM_DIR}/etc/bashrc"

# Fetch and install OpenFOAM's third-party dependencies
wget -O - http://dl.openfoam.org/third-party/10 | tar xvz
mv "ThirdParty-10-version-10" "${THIRDPARTY_DIR}"
(
    cd "${THIRDPARTY_DIR}" \
    && echo ./Allwmake
)

# Refresh OpenFOAM paths
wmRefresh

# Build OpenFOAM
(
    cd "${OPENFOAM_DIR}" \
    && ./Allwmake -j
)
