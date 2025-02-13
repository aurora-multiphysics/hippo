# hippo

Hippo is a MOOSE app wrapping OpenFOAM's fluid solver.
It provides tools for solving coupled conjugate heat transfer problems.

Some simple 1 and 2D validation cases can be found in
[the tests](https://github.com/aurora-multiphysics/hippo/tree/main/test/tests/multiapps).

## Install/Build

Note: Only tested with GCC.

### OpenFOAM

Unfortunately hippo relies on patching OpenFOAM,
there is a script `scripts/install-openfoam.sh` that will clone
OpenFOAM-10, apply the patch, and build it.

First install the build dependencies:

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

Then run the script (use flag `-h` for help, this will take ~1 hour):

```bash
bash ./scripts/install-openfoam.sh
```

To set the OpenFOAM build options (e.g., Opt or Debug mode),
copy `scripts/openfoam-prefs.sh` to `~/.OpenFOAM/prefs.sh`,
and update the environment variables within.
The variables will be loaded when `<path/to/OpenFOAM>/etc/bashrc` is sourced
(which is done automatically by the install script).

### Moose

Follow instructions
[here](https://mooseframework.inl.gov/getting_started/installation/gcc_install_moose.html)
to build MOOSE.

Hippo requires MOOSE release `2024-11-11-release` or higher.

### Environment

Set (recommend [direnv](https://direnv.net/) for this)

```bash
MOOSE_DIR=/path/to/moose/dir
MPI_ROOT=/path/to/mpich/root
MPI_ARCH_INC="-I/path/to/mpi/includes"
MPI_ARCH_LIBS="-L/path/to/mpi/libs -lmpi"
source /path/to/OpenFOAM/etc/bashrc
```

### Build

Once the dependencies are built, then (in theory) you should be able to `make` in the root directory:

```bash
make
```

Or, for a debug build:

```bash
METHOD=dbg make
```

## Tests

Hippo's tests are written using MOOSE's
[test harness system](https://mooseframework.inl.gov/python/TestHarness.html).
Just run `./run_tests`.

Note that you must have the required Python packages installed:

```bash
pip install -r requirements.test.txt
```

## Quality

Some code quality checks are set up with
[`pre-commit`](https://pre-commit.com/).

Install the pre-commit hooks to run on each new commit:

```bash
pre-commit install
```

Run the hooks manually:

```bash
pre-commit run --all
```

To bypass the pre-commit checks, use the `--no-verify` (or `-n`) flag:

```bash
git commit --no-verify
```

## Notes

This is a work in progress.
So far, Hippo can:

- Create a 2D MOOSE boundary mesh from the boundary of an OpenFOAM mesh.
- Run a fluid solver case via a MOOSE input file.
- Transfer temperatures and or heat fluxes from an OpenFOAM mesh to
  the MOOSE boundary mesh.
- Set OpenFOAM temperature field boundary conditions
  using values from a MOOSE mesh.

### Floating Point Error Trapping

If you have floating point error (FPE) trapping enabled in OpenFOAM,
you may run into errors when running Hippo.

Hippo input files that run a MOOSE case do not require a Kernel or variables.
However, MOOSE will still happily go off and calculate a residual.
Since there are no variables defined,
the residual calculation results in a division by zero and an FPE signal.
OpenFOAM will catch this signal and abort the application.

Either of these two workarounds will work:

1. Disable trapping for floating point exceptions.
   `unset FOAM_SIGFPE && unset FOAM_SETNAN`.

2. Add a dummy variable to the MOOSE input file.
   E.g.,

   ```toml
   [Variables]
     [dummy]
       initial_value = 999
     []
   []
   ```
