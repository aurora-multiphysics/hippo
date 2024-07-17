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

CASE_DIR = Path(__file__).parent
FOAM_DIR = CASE_DIR / "buoyantCavity"

# Are we in cm or m?

moose_cp = 420  # J/kg/K
moose_rho = 0.2381  # kg/m3
moose_vol = 1 * 0.25 * 0.5  # 0.125 m3 (?)
moose_mass = moose_rho * moose_vol  # kg
moose_mesh = ExodusIIFile(str(CASE_DIR / "run_out.e"))
moose_t0 = moose_mesh.get_node_variable_values("temp",  1)
n_time_steps = moose_mesh.get_times().size
moose_t = moose_mesh.get_node_variable_values("temp",  n_time_steps)
moose_dt = moose_t - moose_t0
moose_de = moose_mass * moose_cp * moose_dt.sum()  # J

foam_cp = 1004.4  # J/kg/K
foam_vol = 3.5 * 0.5 * 0.5  # 0.875 m3 (?)
foam_n_elements = 1050
foam_rho = ff.readfield(str(FOAM_DIR), time_name="1", name="rho", verbose=False)  # kg/m3
foam_mass = foam_rho * foam_vol/foam_n_elements  # kg for each element
foam_t0 = ff.readfield(str(FOAM_DIR), time_name="0", name="T", verbose=False)
foam_t = ff.readfield(str(FOAM_DIR), time_name="1", name="T", verbose=False)
foam_dt = foam_t - foam_t0  # for each element
foam_de = (foam_mass * foam_cp * foam_dt).sum()  # J

e_dt = np.abs(foam_de - moose_de)

print("MOOSE:        Q = m.c.ΔT =", f"{moose_de:e}")
print("OpenFOAM:     Q = m.c.ΔT =", f"{foam_de:e}")
print("Energy delta:         ΔQ =", f"{e_dt:e}")
