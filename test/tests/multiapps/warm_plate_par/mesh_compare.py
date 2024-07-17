"""
A script to compare OpenFOAM and MOOSE meshes.

Requirements:

    pip install \\
        fluidfoam \\
        git+https://github.com/sandialabs/exodusii@2024.01.09 \\

"""

import warnings
from pathlib import Path

import fluidfoam as ff
import numpy as np
from exodusii.file import ExodusIIFile

warnings.filterwarnings("ignore", module=".*netcdf.*")

CASE_DIR = Path("/home/bf2936/hippo/test/tests/multiapps/warm_plate")
FOAM_DIR = CASE_DIR / "hotRoom"
WHF = "wallHeatFlux"
FOAM_PATCH = "ceiling"

for time_step_idx, time in enumerate((str(x/10) for x in range(0, 50, 2))):
    if int(float(time)) == float(time):
        time = str(int(float(time)))

    # Read OpenFOAM mesh
    patch = ff.readfield(
        str(FOAM_DIR), time_name=time, name=WHF, boundary=FOAM_PATCH
    )

    # Read MOOSE mesh
    moose_mesh = ExodusIIFile(str(CASE_DIR / "run_out.e"))
    moose_patch = moose_mesh.get_element_variable_values(
        0, "foamT_face", time_step_idx + 1
    )

    if not np.allclose(patch, moose_patch):
        print(f"({time_step_idx} {time}) not close")
    # np.testing.assert_allclose(
    #     patch,
    #     moose_front_and_back,
    #     err_msg=f"time step {time} failed",
    # )
