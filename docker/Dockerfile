# Docker file that can be used to build/run Hippo.
#
# This does not add a copy of hippo into the container. The expectation
# is that the user mounts a local clone of hippo when launching the
# container:
#
#   docker run -ti -v <path/to/hippo>:/opt/hippo/ hippo /bin/bash
#
# This container is not intended to be used as a development
# environment, but it does provide a stable place to build and run
# hippo (for example in a CI pipeline).

FROM idaholab/moose:aff4e7127b5492794595c14a49115b75bd159b31

WORKDIR "/opt/hippo"
COPY ./scripts/install-openfoam.sh ./scripts/install-openfoam.sh
COPY ./scripts/openfoam.patch ./scripts/openfoam.patch
COPY ./scripts/openfoam-prefs.sh /root/.OpenFOAM/prefs.sh

ENV MOOSE_DIR=/opt/moose
ENV MPI_ROOT=/usr/local
ENV MPI_ARCH_INC="-I/usr/local/include"
ENV MPI_ARCH_LIBS="-L/usr/local/lib -lmpi"
ENV CC=/usr/local/bin/mpicc
ENV CXX=/usr/local/bin/mpicxx
ENV F77=/usr/local/bin/mpif77
ENV F90=/usr/local/bin/mpif90

RUN DEBIAN_FRONTEND=noninteractive \
    # Install OpenFOAM build dependencies
    apt-get update && \
    apt-get install -y --no-install-recommends \
        libqt5opengl5-dev \
        libqt5x11extras5-dev \
        libxt-dev \
        paraview \
        paraview-dev \
        qtbase5-dev \
        qttools5-dev \
        qttools5-dev-tools \
        wget \
    && \
    # Install OpenFOAM
    bash ./scripts/install-openfoam.sh -o /opt/openfoam -s && \
    # The MOOSE container leaves an mpich tar lying around...
    rm "$(find /opt -maxdepth 1 -type f -name 'mpich-*')" && \
    # Clean up build dependencies
    apt-get -y remove \
        libqt5opengl5-dev \
        libqt5x11extras5-dev \
        libxt-dev \
        paraview \
        paraview-dev \
        qtbase5-dev \
        qttools5-dev \
        qttools5-dev-tools \
        wget \
    && \
    rm -rf /var/lib/apt/lists/*

ENTRYPOINT ["/bin/bash", "-c", "source /opt/openfoam/OpenFOAM-10/etc/bashrc && \"$@\"", "-s"]
