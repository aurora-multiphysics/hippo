# Developer Guide

This page contains information to aid in development of Hippo.

## Docker

The Dockerfile `docker/Dockerfile` contains the build definition for
Hippo's CI environment.
This environment does not contain Hippo itself,
as it is used for testing the build.

### Quay.io

The built docker image is stored under
[UKAEA's Quay.io organisation](https://quay.io/repository/ukaea/hippo).
A [robot account](https://docs.quay.io/glossary/robot-accounts.html)
has been set up with write permissions for the Hippo repository.
This robot account is used to authenticate between Quay and GitHub.
Contact UKAEA's Quay.io admins (or the RSE team) for support with this account.
