FROM idaholab/moose:aff4e7127b5492794595c14a49115b75bd159b31

WORKDIR "/opt/hippo"
COPY ./scripts/install-openfoam.sh ./scripts/install-openfoam.sh
COPY ./scripts/openfoam.patch ./scripts/openfoam.patch
COPY ./scripts/openfoam-prefs.sh /root/.OpenFOAM/prefs.sh

# Use a bash shell or OpenFOAM's bashrc won't work properly
RUN DEBIAN_FRONTEND=noninteractive \
    apt-get update && \
    apt-get install -y --no-install-recommends \
        libqt5opengl5-dev=5.12.8+dfsg-0ubuntu2.1 \
        libqt5x11extras5-dev=5.12.8-0ubuntu1 \
        libxt-dev=1:1.1.5-1 \
        paraview=5.7.0-4ubuntu9 \
        paraview-dev=5.7.0-4ubuntu9 \
        qtbase5-dev=5.12.8+dfsg-0ubuntu2.1 \
        qttools5-dev=5.12.8-0ubuntu1 \
        qttools5-dev-tools=5.12.8-0ubuntu1 \
        wget=1.20.3-1ubuntu2 \
    && \
    # Install OpenFOAM
    bash ./scripts/install-openfoam.sh -o /opt/openfoam -s && \
    # Remove the bits of OpenFOAM we don't need
    rm -rf /opt/openfoam/OpenFOAM-10/Allwmake && \
    rm -rf /opt/openfoam/OpenFOAM-10/.git/ && \
    rm -rf /opt/openfoam/OpenFOAM-10/.gitattributes/ && \
    rm -rf /opt/openfoam/OpenFOAM-10/.gitignore/ && \
    rm -rf /opt/openfoam/OpenFOAM-10/applications/ && \
    rm -rf /opt/openfoam/OpenFOAM-10/doc/ && \
    rm -rf /opt/openfoam/OpenFOAM-10/test/ && \
    rm -rf /opt/openfoam/OpenFOAM-10/tutorials/ && \
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

ENV MOOSE_DIR=/opt/moose
ENTRYPOINT ["/bin/bash", "-c", "source /opt/openfoam/OpenFOAM-10/etc/bashrc && \"$@\"", "-s"]
