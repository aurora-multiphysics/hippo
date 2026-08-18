#!/usr/bin/env bash
set -euo pipefail

version="4.10.0"
prefix="/opt/findutils-${version}"
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

mkdir -p /usr/local/bin
ln -sfn "${prefix}/bin/find" /usr/local/bin/find

# Verify the comma-separated type syntax used by OpenFOAM's CleanFunctions.
/usr/local/bin/find --version
/usr/local/bin/find /tmp -maxdepth 0 -type f,l -print
