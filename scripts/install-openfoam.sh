#!/usr/bin/env bash
# shellcheck disable=SC1091

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
STRIP_SOURCES=0
OUT_DIR="$(dirname "${SCRIPT_DIR}")/external/openfoam"
USAGE="usage: install-openfoam.sh [-h] [-s] [-o DIRECTORY]

Install OpenFOAM-10 for Ubuntu, applying the patch required by hippo.

Note that you will need to install OpenFOAM's requirements separately.
You can do this by running:

    apt install \\
        libqt5opengl5-dev \\
        libqt5x11extras5-dev \\
        libxt-dev \\
        mpich \\
        paraview \\
        paraview-dev \\
        qtbase5-dev \\
        qttools5-dev \\
        qttools5-dev-tools

options:
  -o <DIRECTORY>  the directory to install OpenFOAM within. This will be
                  created if it doesn't exist
                  [default: '${OUT_DIR}']
  -s              if given, remove all OpenFOAM sources, keeping only the build
                  artifacts
  -h              show help and exit
"

SCRIPT_REQUIREMENTS=("getopts" "tar")
for REQ in "${SCRIPT_REQUIREMENTS[@]}"; do
    if ! command -v "${REQ}" > /dev/null; then
        >&2 echo "install-openfoam: error: '${REQ}' is not installed and is required to run this script"
        exit 1
    fi
done

while getopts "o:sh" opt; do
    case "${opt}" in
        o) OUT_DIR="${OPTARG}" ;;
        s) STRIP_SOURCES=1 ;;
        h) echo "${USAGE}" && exit 0 ;;
        *) exit 1
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
    && ./Allwmake -j -s -q -l
)

if [ ${STRIP_SOURCES} -eq 1 ]; then
    rm -rf "${OPENFOAM_DIR}/OpenFOAM-10/wmake"
    rm -rf "${OPENFOAM_DIR}/OpenFOAM-10/doc"
    rm -rf "${OPENFOAM_DIR}/OpenFOAM-10/src"
    rm -rf "${OPENFOAM_DIR}/OpenFOAM-10/test"
    rm -rf "${OPENFOAM_DIR}/OpenFOAM-10/tutorials"
fi
