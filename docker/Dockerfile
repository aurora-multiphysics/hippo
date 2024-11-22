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

FROM idaholab/moose-dev:e930b1d

WORKDIR "/opt/hippo"
COPY ./scripts/install-openfoam.sh ./scripts/install-openfoam.sh
COPY ./scripts/openfoam.patch ./scripts/openfoam.patch
COPY ./scripts/openfoam-prefs.sh /root/.OpenFOAM/prefs.sh

ENV MOOSE_DIR=/opt/moose
ENV MPI_ROOT=/opt/mpich
ENV MPI_ARCH_INC="-I/opt/mpich/include"
ENV MPI_ARCH_LIBS="-L/opt/mpich/lib -lmpi"
ENV CC=/opt/mpich/bin/mpicc
ENV CXX=/opt/mpich/bin/mpicxx
ENV F77=/opt/mpich/bin/mpif77
ENV F90=/opt/mpich/bin/mpif90

# Build MOOSE.
RUN git clone --branch 2024-11-11-release https://github.com/idaholab/moose.git "${MOOSE_DIR}"
RUN . /environment && make -C "${MOOSE_DIR}/test/"
RUN . /environment && make -C "${MOOSE_DIR}/modules/heat_transfer/"
# Build OpenFOAM.
RUN . /environment && bash ./scripts/install-openfoam.sh -o /opt/openfoam -s
# Python requirements for linting and testing.
RUN . /environment && conda install -y fluidfoam pre-commit

ENTRYPOINT ["/bin/bash", "-c", "source /opt/openfoam/OpenFOAM-10/etc/bashrc && \"$@\"", "-s"]
