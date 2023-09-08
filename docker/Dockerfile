FROM idaholab/moose:aff4e7127b5492794595c14a49115b75bd159b31

# Use a bash shell or OpenFOAM's bashrc won't work properly
RUN DEBIAN_FRONTEND=noninteractive \
    apt-get update \
    && apt-get install -y --no-install-recommends \
        libqt5opengl5-dev=5.15.3+dfsg-2ubuntu0.2 \
        libqt5x11extras5-dev=5.15.3-1 \
        libxt-dev=1:1.2.1-1 \
        mpich=4.0-3 \
        paraview=5.10.0~rc1-1build3 \
        paraview-dev=5.10.0~rc1-1build3 \
        qtbase5-dev=5.15.3+dfsg-2ubuntu0.2 \
        qttools5-dev=5.15.3-1 \
        qttools5-dev-tools=5.15.3-1 \
        wget=1.21.2-2ubuntu1 \
    && rm -rf /var/lib/apt/lists/*

ENV MPI_ROOT="/usr/lib/x86_64-linux-gnu/mpich" \
    MPI_ARCH_INC="-I/usr/lib/x86_64-linux-gnu/mpich/include/" \
    MPI_ARCH_LIBS="-L/usr/lib/x86_64-linux-gnu -lmpi"

WORKDIR "/opt/hippo"
COPY ./scripts/install-openfoam.sh ./scripts/install-openfoam.sh
COPY ./scripts/openfoam.patch ./scripts/openfoam.patch

# Install OpenFOAM
RUN bash ./scripts/install-openfoam.sh -o /opt/openfoam

# Set required environment variables
ENV MOOSE_DIR=/opt/moose
ENTRYPOINT ["/bin/bash", "-c", "source /opt/openfoam/OpenFOAM-10/etc/bashrc && \"$@\"", "-s"]
