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

FROM idaholab/moose-dev:2025.05.23 AS moose-deps

# Strip debug libraries out of libMesh
RUN . /environment \
    && for method in oprof devel dbg; do \
        find ${LIBMESH_DIR}/lib -name "*_$method.so*" -delete -print; \
        find ${LIBMESH_DIR}/lib -name "*_$method.la" -delete -print; \
        find ${LIBMESH_DIR}/bin -name "*-$method" -delete -print; \
        find ${LIBMESH_DIR} -name "*-$method.pc" -delete -print; \
    done

FROM rockylinux/rockylinux:8.10 AS main

# Build args, set with:
# docker build --build-arg "MOOSE_JOBS=4" --build-arg "OPENFOAM_JOBS=8" ...
# Args must be in the same image in multi-stage build
# Allow specifying UID and GID to avoid permissions problems when mounting directories
ARG UID=1001
ARG GID=1001
ARG UNAME=hippo-user
# Number of parallel jobs for MOOSE build
ARG MOOSE_JOBS=1
# Number of parallel jobs for OPENFOAM build, 0 for no limit
ARG OPENFOAM_JOBS=0

COPY --from=moose-deps /opt/libmesh /opt/libmesh
COPY --from=moose-deps /opt/rh /opt/rh
COPY --from=moose-deps /opt/petsc /opt/petsc
COPY --from=moose-deps /opt/wasp /opt/wasp
COPY --from=moose-deps /opt/openmpi /opt/openmpi
COPY --from=moose-deps /opt/miniforge3 /opt/miniforge3
COPY --from=moose-deps /opt/vtk /opt/vtk
COPY --from=moose-deps /lib64/libmpc* /lib64

# Dependencies
RUN dnf --enablerepo=devel install -y \
        findutils \
        flex \
        git \
        glibc-devel \
        glibc-headers \
        libgfortran \
        libgomp \
        libtirpc-devel \
        make \
        wget \
        zlib-devel \
    && wget https://dl.fedoraproject.org/pub/epel/8/Everything/x86_64/Packages/g/glpk-4.65-4.el8.x86_64.rpm \
    && wget https://dl.fedoraproject.org/pub/epel/8/Everything/x86_64/Packages/g/glpk-devel-4.65-4.el8.x86_64.rpm \
    && dnf install -y \
        ./glpk-4.65-4.el8.x86_64.rpm \
        ./glpk-devel-4.65-4.el8.x86_64.rpm \
    && rm glpk-* \
    && dnf clean all

# MOOSE
ENV MOOSE_DIR=/opt/moose
ENV LIBMESH_DIR=/opt/libmesh
ENV PETSC_DIR=/opt/petsc
ENV WASP_DIR=/opt/wasp
ENV VTKINCLUDE_DIR=/opt/vtk/include/vtk-9.3
ENV VTKLIB_DIR=/opt/vtk/lib
# MPI
ENV MPI_ROOT=/opt/openmpi
ENV MPI_ARCH_INC="-I/opt/openmpi/include"
ENV MPI_ARCH_LIBS="-L/opt/openmpi/lib -lmpi"
ENV LD_LIBRARY_PATH="${MPI_ROOT}/lib:${LD_LIBRARY_PATH}"
ENV PATH="${MPI_ROOT}/bin:${PATH}"
# Compiler
ENV CC=/opt/openmpi/bin/mpicc
ENV CXX=/opt/openmpi/bin/mpicxx
ENV F77=/opt/openmpi/bin/mpif77
ENV F90=/opt/openmpi/bin/mpif90
ENV PATH="/opt/rh/gcc-toolset-12/root/usr/bin:${PATH}"
ENV CPLUS_INCLUDE_PATH="/opt/rh/gcc-toolset-12/root/usr/include/c++/12:${CPLUS_INCLUDE_PATH}"
ENV LD_LIBRARY_PATH="/opt/rh/gcc-toolset-12/root/usr/lib64:/opt/rh/gcc-toolset-12/root/usr/lib:${LD_LIBRARY_PATH}"
# Conda
ENV CONDA_PREFIX=/opt/miniforge3
ENV CONDA_SHLVL=1
ENV CONDA_EXE=/opt/miniforge3/bin/conda
ENV CONDA_PYTHON_EXE=/opt/miniforge3/bin/python
ENV CONDA_DEFAULT_ENV=base
ENV PATH="${CONDA_PREFIX}/bin:/opt/miniforge3/condabin:${PATH}"
ENV CPLUS_INCLUDE_PATH="/opt/miniforge3/include/python3.12:${CPLUS_INCLUDE_PATH}"
RUN ln -s /opt/miniforge3/bin/python3-config /opt/miniforge3/bin/python-config

WORKDIR "/opt/hippo"

# Build MOOSE
RUN git clone --branch 2025-09-05-release https://github.com/idaholab/moose.git "${MOOSE_DIR}"
RUN make -C "${MOOSE_DIR}/modules/heat_transfer" -j $MOOSE_JOBS

# Build OpenFOAM
# Ugly, but the Zoltan build looks for things in 'lib64' that are in 'lib'
RUN ln -s "${MPI_ROOT}/lib" "${MPI_ROOT}/lib64"
COPY ./scripts/install-openfoam.sh ./scripts/install-openfoam.sh
COPY ./scripts/openfoam.patch ./scripts/openfoam.patch
COPY ./scripts/openfoam-prefs.sh /root/.OpenFOAM/prefs.sh
RUN bash ./scripts/install-openfoam.sh -o /opt/openfoam -s -j$OPENFOAM_JOBS -q

# Python linting and testing requirements
COPY ./requirements.test.txt ./requirements.test.txt
RUN conda install pre-commit --file requirements.test.txt -S --yes

# Pull down MOOSE's 'large_media' submodule for building docs
RUN git -C "${MOOSE_DIR}" submodule update --init large_media

RUN groupadd -g $GID --non-unique $UNAME
RUN useradd -u $UID -g $GID --shell /bin/bash --create-home $UNAME
RUN chown -R $UNAME:$UNAME /opt/moose
USER $UNAME
ENV USER $UNAME
WORKDIR /home/$UNAME

ENTRYPOINT ["/bin/bash", "-c", "source /opt/openfoam/OpenFOAM-12/etc/bashrc && \"$@\"", "-s"]
