#!/usr/bin/env bash
set -euo pipefail

version="4.10.0"
mkdir -p /usr/local/bin
prefix="/usr/local"
build_dir="$(mktemp --tmpdir --directory findutils-build.XXXXXX)"

cleanup()
{
    rm -rf "${build_dir}"
}
trap cleanup EXIT

cd "${build_dir}"
wget --quiet "https://ftp.gnu.org/gnu/findutils/findutils-${version}.tar.xz"
tar -xf "findutils-${version}.tar.xz"
cd "findutils-${version}"

./configure \
    --prefix="${prefix}" \
    --disable-nls

make -j"$(nproc)"
make install
