# Installation

Hippo's source code can be cloned from GitHub:

```bash
git clone https://github.com/aurora-multiphysics/hippo.git
```

## Dependencies

First, install the build dependencies

```bash
apt install \
    bison \
    flex \
    libptscotch-dev \
    libqt5opengl5-dev \
    libqt5x11extras5-dev \
    libxt-dev \
    make \
    paraview \
    paraview-dev \
    qtbase5-dev \
    qttools5-dev \
    qttools5-dev-tools
```

## Installing OpenFOAM

Hippo currently works with openfoam.org (The OpenFOAM Foundation's)
flavour of OpenFOAM and has been tested on OpenFOAM-12 specifically.
Unfortunately, for Hippo to work correctly, OpenFOAM needs to be patched.
For convenience, the script [`scripts/install-openfoam.sh`](https://github.com/aurora-multiphysics/hippo/blob/main/scripts/install-openfoam.sh)
has been provided to patch and install OpenFOAM-12.
To execute the script, run

```bash
bash ./scripts/install-openfoam.sh
```

The `-h` flag shows the installation options.
To set the OpenFOAM build options (e.g., Opt or Debug mode),
copy `scripts/openfoam-prefs.sh` to `~/.OpenFOAM/prefs.sh`,
and update the environment variables within.
The variables will be loaded when `<path/to/OpenFOAM>/etc/bashrc` is sourced
(which is done automatically by the install script).

## Installing MOOSE

Follow instructions
[here](https://mooseframework.inl.gov/getting_started/installation/gcc_install_moose.html)
to build MOOSE.

Hippo requires MOOSE release `2024-09-05-release` or higher.

## Environment

Set (recommend [direnv](https://direnv.net/) for this)

```bash
MOOSE_DIR=/path/to/moose/dir
MPI_ROOT=/path/to/mpich/root
MPI_ARCH_INC="-I/path/to/mpi/includes"
MPI_ARCH_LIBS="-L/path/to/mpi/libs -lmpi"
source /path/to/OpenFOAM/etc/bashrc
```

## Building Hippo

Once the dependencies are built, then (in theory) you should be able to `make` in the root directory:

```bash
make
```

Or, for a debug build:

```bash
METHOD=dbg make
```

The `-j N` option can be used for multi-threaded builds with `N` threads.
