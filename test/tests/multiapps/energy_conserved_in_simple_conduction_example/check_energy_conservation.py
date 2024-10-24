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

# MOOSE heat energy
moose_cp = 200  # J/kg/K
moose_rho = 200  # kg/m3
moose_vol = 1 * 1 * 1  # m3
moose_mass = moose_rho * moose_vol  # kg
moose_mesh = ExodusIIFile(str(CASE_DIR / "run_out.e"))
moose_t0 = moose_mesh.get_node_variable_values("temp", 1)
n_time_steps = moose_mesh.get_times().size
moose_t = moose_mesh.get_node_variable_values("temp", n_time_steps)
moose_dt = moose_t - moose_t0
moose_de = moose_mass / len(moose_t) * moose_cp * moose_dt.sum()  # J


def foam_energy_at_time(case_dir: str, time_name: str) -> float:
    """
    Calculate the energy in an OpenFOAM fluid mesh at a time step.

    Uses

        Q = (m.Cp.ΔT) + (½.m.|U.U|) + (m.g.h)
            ----+----   -----+-----   ---+---
             internal      kinetic      grav.
              energy                   potential
    Where
        Cp is specific heat capacity,
        U is velocity,
        g is acceleration due to gravity,
        h is height,
        m is mass.

    Note that in this script we only care about changes in energy.
    """
    G = 9.81  # m/s2
    VOL = 1 * 1 * 1  # m3
    mesh = ff.readmesh(case_dir, verbose=False)
    n_cells = len(mesh[0])
    rho = ff.readfield(case_dir, time_name=time_name, name="rho", verbose=False)
    if len(rho) == 1:
        rho = np.full(n_cells, rho)
    cell_volume = VOL / n_cells
    mass = rho * cell_volume  # mass for each cell volume
    u = ff.readfield(case_dir, time_name=time_name, name="U", verbose=False)
    y_coord = mesh[1]
    t = ff.readfield(case_dir, time_name=time_name, name="T", verbose=False)

    # Energy components for each cell volume
    cp = 1500.0
    internal_energy = mass * cp * t
    kinetic_energy = 0.5 * mass * np.sum(u * u, axis=0) ** 2
    grav_potential_energy = mass * G * y_coord
    print(f"{internal_energy.sum()=:e}")
    print(f"{kinetic_energy.sum()=:e}")
    print(f"{grav_potential_energy.sum()=:e}")
    return (internal_energy + kinetic_energy + grav_potential_energy).sum()


foam_ts_0 = "0"
foam_ts = "100"
foam_energy_0 = np.abs(foam_energy_at_time(str(FOAM_DIR), foam_ts_0))
print()
foam_energy = np.abs(foam_energy_at_time(str(FOAM_DIR), foam_ts))
print()
foam_de = foam_energy - foam_energy_0

print("MOOSE:                          ΔQ = m.c.ΔT =", f"{moose_de:e}")
print("OpenFOAM: ΔQ = m.Cp.ΔT + ½.m.Δ|U.U| + m.g.h =", f"{foam_de:e}")
print(
    "Energy delta:                         Δ(ΔQ) =",
    f"{abs(moose_de) - abs(foam_de):e}",
)
