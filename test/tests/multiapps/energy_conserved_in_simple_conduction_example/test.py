import unittest
from pathlib import Path

import fluidfoam as ff
import numpy as np

CASE_DIR = Path(__file__).parent
FOAM_DIR = CASE_DIR / "buoyantCavity"
MOOSE_CP = 200
MOOSE_DENSITY = 200


class TestEnergyConservedInSimpleConductionExample(unittest.TestCase):
    def test_moose_and_foam_energy_deltas_are_equal(self):
        moose_csv_data = np.genfromtxt(
            CASE_DIR / "run_out.csv", delimiter=",", skip_header=1
        )
        moose_timesteps = moose_csv_data[:, 0]
        moose_temp_integral = moose_csv_data[:, 1]

        initial_t = 20
        initial_t_idx = np.where(np.isclose(moose_timesteps, initial_t))[0][0]
        initial_moose_temp = moose_temp_integral[initial_t_idx]
        initial_moose_e = MOOSE_DENSITY * MOOSE_CP * initial_moose_temp
        initial_foam_e = foam_energy_at_time(str(FOAM_DIR), str(int(initial_t)))

        final_t = moose_timesteps[-1]
        final_t_idx = np.where(np.isclose(moose_timesteps, final_t))[0][0]
        final_moose_temp = moose_temp_integral[final_t_idx]
        final_moose_e = MOOSE_DENSITY * MOOSE_CP * final_moose_temp
        final_foam_e = foam_energy_at_time(str(FOAM_DIR), str(int(final_t)))

        moose_de = final_moose_e - initial_moose_e
        foam_de = final_foam_e - initial_foam_e

        self.assertAlmostEqual(foam_de, -moose_de)


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
    CP = 1500.0  # J/kg/K
    mesh = ff.readmesh(case_dir, verbose=False)
    n_cells = len(mesh[0])
    rho = ff.readfield(case_dir, time_name=time_name, name="rho", verbose=False)
    if len(rho) == 1:
        rho = np.full(n_cells, rho)
    cell_volume = VOL / n_cells
    mass = rho * cell_volume  # mass for each cell volume
    u = ff.readfield(case_dir, time_name=time_name, name="U", verbose=False)
    if len(u) == 0:
        u = np.full(n_cells, u)
    y_coord = mesh[1]
    t = ff.readfield(case_dir, time_name=time_name, name="T", verbose=False)

    # Energy components for each cell volume
    internal_energy = mass * CP * t
    kinetic_energy = 0.5 * mass * np.linalg.norm(u) ** 2
    grav_potential_energy = mass * G * y_coord
    return (internal_energy + kinetic_energy + grav_potential_energy).sum()


if __name__ == "__main__":
    import matplotlib.pyplot as plt

    moose_csv_data = np.genfromtxt(
        CASE_DIR / "run_out.csv", delimiter=",", skip_header=1
    )
    time_steps = moose_csv_data[:, 0]
    moose_temp_integral = moose_csv_data[:, 1]

    # Start from the first non-zero time step to avoid noise.
    initial_t = 20
    idx = np.where(np.isclose(initial_t, time_steps))[0][0]
    time_steps = time_steps[idx:]
    moose_temp_integral = moose_temp_integral[idx:]

    moose_e0 = MOOSE_CP * MOOSE_DENSITY * moose_temp_integral[0]
    foam_e0 = foam_energy_at_time(str(FOAM_DIR), "0")

    foam_t = []
    foam_de = []
    moose_de = []
    for t, moose_temp in zip(time_steps[1:], moose_temp_integral):
        moose_e = MOOSE_CP * MOOSE_DENSITY * moose_temp
        moose_de.append(moose_e - moose_e0)
        if Path(FOAM_DIR / str(int(t))).is_dir():
            foam_t.append(int(t))
            foam_e = foam_energy_at_time(str(FOAM_DIR), str(int(t)))
            foam_de.append(foam_e - foam_e0)

    _, axs = plt.subplots(ncols=2, nrows=2)
    axs[0][0].plot(foam_t, foam_de, label="OpenFOAM")
    axs[0][0].set_xlabel(r"$t$ [s]")
    axs[0][0].set_ylabel(r"$\Delta E$ [J]")
    axs[0][0].legend()

    axs[0][1].plot(time_steps[1:], moose_de, label="MOOSE", color="orange")
    axs[0][1].set_xlabel(r"$t$ [s]")
    axs[0][1].set_ylabel(r"$\Delta E$ [J]")
    axs[0][1].invert_yaxis()
    axs[0][1].legend()

    axs[1][0].plot(foam_t, np.abs(foam_de), label="OpenFOAM")
    axs[1][0].plot(time_steps[1:], np.abs(moose_de), label="MOOSE")
    axs[1][0].set_xlabel(r"$t$ [s]")
    axs[1][0].set_ylabel(r"$|\Delta E|$ [J]")
    axs[1][0].legend()

    ratios = np.abs(foam_de) / np.abs(moose_de[:: len(moose_de) // len(foam_de)])
    axs[1][1].plot(foam_t, ratios, label="Ratio")
    axs[1][1].set_xlabel(r"$t$ [s]")
    axs[1][1].set_ylabel(r"$\frac{T_{\text{foam}}}{T_{\text{moose}}}$")
    axs[1][1].legend()

    plt.show()
