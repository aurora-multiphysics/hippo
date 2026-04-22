# Apptainer

We make use of [Apptainer](https://apptainer.org/docs/user/main/) containers to test Hippo within the CI pipeline and in the future, we intend to include them as part of Hippo releases.
There are two containers
- **Development container:** this contains all of Hippo's dependencies but not Hippo itself.
The definition file can be found [here](https://github.com/aurora-multiphysics/hippo/apptainer/hippo-dev.def).
Stored under
[UKAEA's Quay.io organisation](https://quay.io/repository/ukaea/hippo)
with name `hippo:dev-` followed by the first 10 characters of the Git revision that created it.
- **Release container:** used to build and test Hippo within the CI environment.
The definition file can be found [here](https://github.com/aurora-multiphysics/hippo/apptainer/hippo-release.def).


## Building the containers

To build the release container locally, run (from the root directory)

```bash
apptainer build hippo-release.sif apptainer/hippo-release.def
```

## Running the containers

The most common way to use MPI with Apptainer,
is to use the [hybrid approach](https://apptainer.org/docs/user/latest/mpi.html),
in which the host MPI is used to launch the container and works with the MPI installed in the container.
This means that the MPI in the container must be compatible with the host MPI.
The release container currently uses OpenMPI, but in the future we plan to alsp support MPICH.

To run Hippo using the container with MPI on 4 processes

```bash
mpirun -n 4 apptainer run ./hippo-release.sif -i main.i
```

or more simply

```bash
mpirun -n 4 ./hippo-release.sif -i main.i
```

## The CI pipeline

The development container is created on
[workflow dispatch](https://github.com/aurora-multiphysics/hippo/actions/workflows/deploy-apptainer-dev.yaml)
and stored under
[UKAEA's Quay.io organisation](https://quay.io/repository/ukaea/hippo)
with name `hippo:dev-` followed by the first 10 characters of the Git revision that created it.
Once the image has been built and pushed to Quay.io,
a developer should open a PR that updates `.github/workflows/ci.yml`
A [robot account](https://docs.quay.io/glossary/robot-accounts.html)
has been set up with write permissions for the Hippo repository.
This robot account is used to authenticate between Quay and GitHub.
Contact UKAEA's Quay.io admins (or the RSE team) for support with this account.
