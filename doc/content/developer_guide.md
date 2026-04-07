# Developer Guide

This page contains information to aid in development of Hippo.

## Documentation

The project uses the MOOSE docs system to generate its HTML documentation.
The HTML artifacts are generated and deployed within Hippo's CI.
The CI pipeline is run on any pull request
and also on a push to `main` (i.e., a merged pull request).
The documentation pages are always built,
however they are only deployed on a push to `main`.

## Apptainer

The [Apptainer](https://apptainer.org/docs/user/main/) containers are used to test Hippo within the CI pipeline.
There are two containers
- **Development container:** this contains all of Hippo's dependencies but not Hippo itself.
The definition file can be found [here](https://github.com/aurora-multiphysics/hippo/apptainer/hippo-dev.def).
- **Release container:** used to build and test Hippo within the CI environment.
The definition file can be found [here](https://github.com/aurora-multiphysics/hippo/apptainer/hippo-release.def).

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
