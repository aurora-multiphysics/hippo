FROM idaholab/moose:aff4e7127b5492794595c14a49115b75bd159b31

# Use a bash shell or OpenFOAM's bashrc won't work properly
RUN DEBIAN_FRONTEND=noninteractive \
    apt-get update \
    && apt-get install -y --no-install-recommends \
        libqt5opengl5-dev=5.12.8+dfsg-0ubuntu2.1 \
        libqt5x11extras5-dev=5.12.8-0ubuntu1 \
        libxt-dev=1:1.1.5-1 \
        mpich=3.3.2-2build1 \
        paraview=5.7.0-4ubuntu9 \
        paraview-dev=5.7.0-4ubuntu9 \
        qtbase5-dev=5.12.8+dfsg-0ubuntu2.1 \
        qttools5-dev=5.12.8-0ubuntu1 \
        qttools5-dev-tools=5.12.8-0ubuntu1 \
        wget=1.20.3-1ubuntu2 \
    && rm -rf /var/lib/apt/lists/*

ENV MPI_ROOT="/usr/lib/x86_64-linux-gnu/mpich" \
    MPI_ARCH_INC="-I/usr/lib/x86_64-linux-gnu/mpich/include/" \
    MPI_ARCH_LIBS="-L/usr/lib/x86_64-linux-gnu -lmpi"

WORKDIR "/opt/hippo"
COPY ./scripts/install-openfoam.sh ./scripts/install-openfoam.sh
COPY ./scripts/openfoam.patch ./scripts/openfoam.patch
COPY ./scripts/openfoam-prefs.sh /root/.OpenFOAM/prefs.sh

RUN bash ./scripts/install-openfoam.sh -o /opt/openfoam -s

ENV MOOSE_DIR=/opt/moose
ENTRYPOINT ["/bin/bash", "-c", "source /opt/openfoam/OpenFOAM-10/etc/bashrc && \"$@\"", "-s"]
