FROM idaholab/moose:latest as openfoam_builder

# Use a bash shell or OpenFOAM's bashrc won't work properly
SHELL ["/bin/bash", "-c"]
ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -y \
    libqt5opengl5-dev \
    libqt5x11extras5-dev \
    libxt-dev \
    mpich \
    paraview \
    paraview-dev \
    qtbase5-dev \
    qttools5-dev \
    qttools5-dev-tools \
    wget
ENV MPI_ROOT=/usr/lib/x86_64-linux-gnu/mpich \
    MPI_ARCH_INC="-I/usr/lib/x86_64-linux-gnu/mpich/include/" \
    MPI_ARCH_LIBS="-L/usr/lib/x86_64-linux-gnu -lmpi"

WORKDIR "/opt/hippo"
COPY ./scripts/install-openfoam.sh ./scripts/install-openfoam.sh
COPY ./scripts/openfoam.patch ./scripts/openfoam.patch

# Install OpenFOAM
RUN bash ./scripts/install-openfoam.sh -o /opt/openfoam

# Set required environment variables
ENV MOOSE_DIR=/opt/moose \
    MPI_ROOT=/usr/lib/x86_64-linux-gnu/mpich \
    MPI_ARCH_INC="-I/usr/lib/x86_64-linux-gnu/mpich/include/" \
    MPI_ARCH_LIBS="-L/usr/lib/x86_64-linux-gnu -lmpi"
ENTRYPOINT ["/bin/bash", "-c", "source /opt/openfoam/OpenFOAM-10/etc/bashrc && \"$@\"", "-s"]
