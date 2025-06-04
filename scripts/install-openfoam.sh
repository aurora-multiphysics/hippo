#!/usr/bin/env bash
# shellcheck disable=SC1091

set -e

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
STRIP_SOURCES=0
BUILD_JOBS=""
OUT_DIR="$(dirname "${SCRIPT_DIR}")/external/openfoam"
USAGE="usage: install-openfoam.sh [-h] [-s] [-o DIRECTORY]

Install OpenFOAM-12 for Ubuntu, applying the patch required by hippo.

Note that you will need to install OpenFOAM's requirements separately.
You can do this by running:

    apt install \\
        build-essential \\
        cmake \\
        flex\\
        gnuplot \\
        gnuplot-x11 \\
        libopenmpi-dev \\
        libxt-dev \\
        zlib1g-dev

options:
  -o <DIRECTORY>  the directory to install OpenFOAM within. This will be
                  created if it doesn't exist
                  [default: '${OUT_DIR}']
  -s              if given, remove OpenFOAM source files not required by
                  hippo
  -j              set the number of build jobs, no limit by default
  -h              show help and exit
"

SCRIPT_REQUIREMENTS=("getopts" "tar" "wget")
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
        j) BUILD_JOBS="${OPTARG}" ;;
        h) echo "${USAGE}" && exit 0 ;;
        *) exit 1
    esac
done

OPENFOAM_DIR="${OUT_DIR}/OpenFOAM-12"
OPENFOAM_REV="9ec94dd57a8d98c3f3422ce9b2156a8b268bbda6"
THIRDPARTY_DIR="${OUT_DIR}/ThirdParty-12"
THIRDPARTY_REV="cab725f5e7929e8f5ec35c54edc493a822355235"

# Fetch and patch OpenFOAM
mkdir -p "${OPENFOAM_DIR}"
if [ ! -d "${OPENFOAM_DIR}/.git" ]; then
    git clone https://github.com/OpenFOAM/OpenFOAM-12.git "${OPENFOAM_DIR}"
fi
git -C "${OPENFOAM_DIR}" reset --hard "${OPENFOAM_REV}"
git -C "${OPENFOAM_DIR}" apply "${SCRIPT_DIR}/openfoam.patch"

# Set up OpenFOAM
source "${OPENFOAM_DIR}/etc/bashrc" || true

echo "Hippo installing OpenFOAM-12 with options:"
echo "------------------------------------------"
echo "  WM_ARCH_OPTION:      ${WM_ARCH_OPTION}"
echo "  WM_COMPILE_OPTION:   ${WM_COMPILE_OPTION}"
echo "  WM_COMPILER_TYPE:    ${WM_COMPILER_TYPE}"
echo "  WM_COMPILER:         ${WM_COMPILER}"
echo "  WM_LABEL_SIZE:       ${WM_LABEL_SIZE}"
echo "  WM_MPLIB:            ${WM_MPLIB}"
echo "  WM_PRECISION_OPTION: ${WM_PRECISION_OPTION}"

mkdir -p "${THIRDPARTY_DIR}"
if [ ! -d "${THIRDPARTY_DIR}/.git" ]; then
    git clone https://github.com/OpenFOAM/ThirdParty-12.git "${THIRDPARTY_DIR}"
fi
git -C "${THIRDPARTY_DIR}" reset --hard "${THIRDPARTY_REV}"
(
    cd "${THIRDPARTY_DIR}" \
    && ./Allwmake
)

# Refresh OpenFOAM paths
wmRefresh || true

# Build OpenFOAM
(
    cd "${OPENFOAM_DIR}" \
    && ./Allwmake -j${BUILD_JOBS} dep \
    && ./Allwmake -j${BUILD_JOBS} src/ \
    && ./Allwmake -j${BUILD_JOBS} applications/modules/ \
    && ./Allwmake -j${BUILD_JOBS} applications/utilities/
)

if [ ${STRIP_SOURCES} -eq 1 ]; then
    rm "${OPENFOAM_DIR}/Allwmake"
    rm "${OPENFOAM_DIR}/.gitattributes"
    rm "${OPENFOAM_DIR}/.gitignore"
    rm -rf "${OPENFOAM_DIR}/.git/"
    rm -rf "${OPENFOAM_DIR}/applications/"
    rm -rf "${OPENFOAM_DIR}/doc/"
    rm -rf "${OPENFOAM_DIR}/platforms/${WM_OPTIONS}/src"
    rm -rf "${OPENFOAM_DIR}/test/"
    rm -rf "${OPENFOAM_DIR}/tutorials/"
    rm -rf "${OPENFOAM_DIR}/wmake/"
fi
