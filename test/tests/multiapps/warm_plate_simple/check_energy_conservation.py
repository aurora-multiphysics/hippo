# """
# A script to compare OpenFOAM and MOOSE meshes.

# Requirements:

#     pip install \\
#         fluidfoam \\
#         git+https://github.com/sandialabs/exodusii@2024.01.09 \\

# """

# import warnings
# from pathlib import Path

# import fluidfoam as ff
# import numpy as np
# from exodusii.file import ExodusIIFile

# warnings.filterwarnings("ignore", module=".*netcdf.*")

# CASE_DIR = Path(__file__).parent
# FOAM_DIR = CASE_DIR / "buoyantCavity"

# moose_cp = 235  # J/kg/K
# moose_rho = 10490  # kg/m3
# moose_vol = 2 * 1 * 1  # 2 m3
# moose_mass = moose_rho * moose_vol  # kg
# moose_mesh = ExodusIIFile(str(CASE_DIR / "run_out.e"))
# moose_t0 = moose_mesh.get_node_variable_values("temp",  1)
# n_time_steps = moose_mesh.get_times().size
# moose_t = moose_mesh.get_node_variable_values("temp",  n_time_steps)
# moose_dt = moose_t - moose_t0
# moose_de = moose_mass * moose_cp * moose_dt.sum()  # J

# # OpenFOAM mass
# DENSITY = "rho"
# foam_rho = ff.readfield(str(FOAM_DIR), time_name="1", name=DENSITY, verbose=False)
# foam_n_cells = foam_rho.size
# foam_rho0 = ff.readfield(str(FOAM_DIR), time_name="0", name=DENSITY, verbose=False)
# if foam_rho0.size == 1:
#     foam_rho0 = np.full(foam_n_cells, foam_rho0[0])
# foam_vol = 2 * 1 * 1  # 2 m3
# foam_m0 = foam_rho0 * foam_vol / foam_n_cells
# foam_m = foam_rho * foam_vol / foam_n_cells
# foam_delta_m = foam_m - foam_m0
# assert np.isclose(foam_m.sum(), foam_m0.sum())
# print(f"{foam_m.sum()=:e}")

# # OpenFOAM Mechanical energy
# VELOCITY = "U"
# foam_u30 = ff.readfield(str(FOAM_DIR), time_name="0", name=VELOCITY, verbose=False)
# foam_u3 = ff.readfield(str(FOAM_DIR), time_name="2", name=VELOCITY, verbose=False)
# foam_uu0 = np.sum(foam_u30*foam_u30, axis=0)  # dot product for each 3-vector
# foam_uu = np.sum(foam_u3*foam_u3, axis=0)
# foam_delta_ek = 0.5 * foam_m * (foam_uu - foam_uu0)
# print(f"{foam_delta_ek.sum()=:e}")

# # OpenFOAM Enthalpy
# ENTHALPY = "Ha"
# foam_h0 = ff.readfield(str(FOAM_DIR), time_name="0", name=ENTHALPY, verbose=False)
# foam_h = ff.readfield(str(FOAM_DIR), time_name="2", name=ENTHALPY, verbose=False)
# foam_delta_h = (foam_h - foam_h0)
# print(f"{foam_delta_h.sum()=:e}")

# foam_de = foam_delta_ek.sum() + (foam_delta_h * foam_m).sum()

# print("MOOSE:                 Q = m.c.ΔT =", f"{moose_de:e}")
# print("OpenFOAM:     Q = m.(H + ½.|U.U|) =", f"{foam_de:e}")
# print("Energy delta:                  ΔQ =", f"{moose_de - foam_de:e}")


"""
A script to compare OpenFOAM and MOOSE meshes.

Requirements:

    pip install \\
        fluidfoam \\
        git+https://github.com/sandialabs/exodusii@2024.01.09 \\

You may also need to run an OpenFOAM post-processor to calculate the
enthalpy at each time step:

    buoyantFoam -postProcess -func totalEnthalpy

"""

import warnings
from pathlib import Path

import fluidfoam as ff
import numpy as np
from exodusii.file import ExodusIIFile

warnings.filterwarnings("ignore", module=".*netcdf.*")

CASE_DIR = Path(__file__).parent
FOAM_DIR = CASE_DIR / "buoyantCavity"

moose_cp = 200  # J/kg/K
moose_rho = 200  # kg/m3
moose_vol = 2 * 1 * 1  # m3
moose_mass = moose_rho * moose_vol  # kg
moose_mesh = ExodusIIFile(str(CASE_DIR / "run_out.e"))
moose_t0 = moose_mesh.get_node_variable_values("temp", 1)
n_time_steps = moose_mesh.get_times().size
moose_t = moose_mesh.get_node_variable_values("temp", n_time_steps)
moose_dt = moose_t - moose_t0
moose_de = moose_mass * moose_cp * moose_dt.sum()  # J

# OpenFOAM Enthalpy
ENTHALPY = "Ha"
foam_h0 = ff.readfield(str(FOAM_DIR), time_name="0", name=ENTHALPY, verbose=False)
foam_h = ff.readfield(str(FOAM_DIR), time_name="1", name=ENTHALPY, verbose=False)
foam_delta_h = foam_h - foam_h0

# Pressure energy (p*V)
PRESSURE = "p"
foam_vol = 2 * 1 * 1  # m3
foam_p0 = ff.readfield(str(FOAM_DIR), time_name="0", name=PRESSURE, verbose=False)
foam_p = ff.readfield(str(FOAM_DIR), time_name="1", name=PRESSURE, verbose=False)
foam_pd = foam_p - foam_p0
foam_delta_pressure_energy = foam_vol * foam_pd

# OpenFOAM mass
foam_n_cells = 2500
foam_rho0 = ff.readfield(str(FOAM_DIR), time_name="0", name="rho", verbose=False)
if foam_rho0.size == 1:
    foam_rho0 = np.full(foam_n_cells, foam_rho0[0])
foam_rho = ff.readfield(str(FOAM_DIR), time_name="1", name="rho", verbose=False)
foam_m0 = foam_rho0 * foam_vol / foam_n_cells
foam_m = foam_rho * foam_vol / foam_n_cells
foam_delta_m = foam_m - foam_m0
assert np.isclose(foam_m.sum(), foam_m0.sum())
print(f"{foam_m.sum()=:e}")

# OpenFOAM Mechanical energy
foam_u30 = ff.readfield(str(FOAM_DIR), time_name="0", name="U", verbose=False)
foam_u3 = ff.readfield(str(FOAM_DIR), time_name="1", name="U", verbose=False)
foam_uu0 = np.sum(foam_u30 * foam_u30, axis=0)  # dot product for each 3-vector
foam_uu = np.sum(foam_u3 * foam_u3, axis=0)
foam_delta_ek = 0.5 * foam_m * (foam_uu - foam_uu0)
print(f"{foam_delta_ek.sum()=:e}")

foam_de = (foam_delta_h - foam_delta_pressure_energy + foam_delta_ek).sum()

print("MOOSE:                         ΔQ = m.c.ΔT =", f"{moose_de:e}")
print("OpenFOAM:     ΔQ = ΔH - Δp.V + ½.m.Δ|U.U| =", f"{foam_de:e}")
print(
    "Energy delta:                        Δ(ΔQ) =", f"{abs(moose_de) - abs(foam_de):e}"
)
