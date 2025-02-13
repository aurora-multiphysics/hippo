!config navigation breadcrumbs=False scrollspy=False

# Hippo

A MOOSE multiapp wrapping OpenFOAM for conjugate heat transfer problems.

## Installation

Build instructions can be found in the repository's
[README](https://github.com/aurora-multiphysics/hippo/blob/main/README.md).

Alternatively, you can use Hippo in a Docker container.
An image with all of Hippo's dependencies pre-installed
can be found on [quay.io](https://quay.io/repository/ukaea/hippo).
You will need to mount and install Hippo within the container.

```shell
$ git clone https://github.com/aurora-multiphysics/hippo.git
$ docker run -it -v $(pwd)/hippo:/opt/hippo quay.io/ukaea/hippo:latest bash
$ make -j
$ hippo-opt --help
```

## Setting Up a Case

See a step-by-step example in [this guide](step_by_step.md).

## Developer Guide

See [here](developer_guide.md) for developer documentation.
