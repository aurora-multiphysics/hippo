FROM ubuntu:24.04

RUN apt-get update
RUN apt-get install -y wget build-essential pkg-config cmake git

# preCICE
RUN wget https://github.com/precice/precice/releases/download/v3.1.2/libprecice3_3.1.2_noble.deb
RUN apt-get install -y ./libprecice3_3.1.2_noble.deb

# OpenFOAM
RUN wget -q -O - https://dl.openfoam.com/add-debian-repo.sh | bash
RUN apt-get install -y openfoam2412-devz

# A place to put the preCICE adapters
WORKDIR /opt/precice-adapters

# OpenFOAM adapter
RUN wget https://github.com/precice/openfoam-adapter/archive/refs/tags/v1.3.1.tar.gz
RUN . /usr/lib/openfoam/openfoam2412/etc/bashrc \
    && tar -xzf v1.3.1.tar.gz \
    && cd openfoam-adapter-1.3.1/ \
    && ./Allwmake -j 12

# Calculix
RUN apt-get install -y libarpack2-dev libspooles-dev libyaml-cpp-dev
RUN cd ~ \
    && wget http://www.dhondt.de/ccx_2.20.src.tar.bz2 \
    && tar xvjf ccx_2.20.src.tar.bz2

# Calculix adapter
RUN git clone --depth 1 --branch v2.20.1 https://github.com/precice/calculix-adapter.git
RUN ADDITIONAL_FFLAGS="--std=legacy" make -C calculix-adapter -j 8
RUN ln -s "$(pwd)/calculix-adapter/bin/ccx_preCICE" /usr/local/bin/ccx_preCICE

# Calculix to paraview mesh converter
RUN apt-get install -y python3.12-pip python3.12-venv libxrender-dev
RUN python3 -m venv /home/root/.venv
RUN . /home/root/.venv/bin/activate && pip install 'ccx2paraview[VTK]'

ENTRYPOINT ["/bin/bash", "-c", "source /usr/lib/openfoam/openfoam2412/etc/bashrc && source /home/root/.venv/bin/activate && \"$@\"", "-s"]
