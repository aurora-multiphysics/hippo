FROM idaholab/moose:latest as openfoam_builder

SHELL ["/bin/bash", "-c"]

ENV MOOSE_DIR=/opt/moose \
    MPI_ROOT=/usr/lib/x86_64-linux-gnu/mpich \
    MPI_ARCH_INC="-I/usr/lib/x86_64-linux-gnu/mpich/include/" \
    MPI_ARCH_LIBS="-L/usr/lib/x86_64-linux-gnu -lmpi"

WORKDIR "/opt/hippo"
COPY ./scripts/install-openfoam.sh ./scripts/install-openfoam.sh
COPY ./scripts/get_openfoam.sh ./scripts/get_openfoam.sh
COPY ./scripts/openfoam.patch ./scripts/openfoam.patch

RUN apt update && apt install -y \
    cmake \
    git \
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

# Install OpenFOAM
RUN bash ./scripts/install-openfoam.sh -o /opt/openfoam

FROM idaholab/moose:latest as main
SHELL ["/bin/bash", "-c"]
COPY --from=openfoam_builder /opt/openfoam /opt/openfoam

USER user
COPY . /opt/hippo
WORKDIR /opt/hippo
RUN source /opt/openfoam/OpenFOAM-10/etc/bashrc

# RUN mkdir -p /opt/OpenFOAM/OpenFOAM-10 \
#     && cd scripts \
#     && ./get_openfoam.sh /opt/OpenFOAM/OpenFOAM-10 \
#     && ls /opt/OpenFOAM/OpenFOAM-10 \
#     && ls /opt/OpenFOAM/OpenFOAM-10/etc \
#     && ls /opt/OpenFOAM/OpenFOAM-10/etc/bashrc \
#     && wget -O - http://dl.openfoam.org/third-party/10 | tar xvz \
#     && mv ThirdParty-10-version-10 /opt/OpenFOAM/ThirdParty-10 \
#     && source /opt/OpenFOAM/OpenFOAM-10/etc/bashrc \
#     && cd /opt/OpenFOAM/ThirdParty-10 \
#     && ./Allwmake \
#     && wmRefresh \
#     && cd /opt/OpenFOAM/OpenFOAM-10/ \
#     && ./Allwmake -j

