"""Test module for the unsteady 1D heat conduction problem"""

import unittest
from pathlib import Path

import numpy as np
import pyvista as pv
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

#import Hippo test python functions
from analytical import unsteady1d_temp
from read_hippo_data import read_moose_exodus_data, read_openfoam_data

RUN_DIR = Path(__file__).parent
FOAM_CASE = RUN_DIR / "fluid-openfoam"
K_SOLID = 1
RHO_CP_SOLID = 1
K_FLUID = 4
RHO_CP_FLUID = 16
T_HOT = 1.
T_COLD = 0.
L = 1


class TestUnsteadyHeatConductionInInfiniteSystem(unittest.TestCase):
    """Test class for 1D unsteady heat conduction problem"""
    def test_matches_analytic_solution_at_times(self):
        """Compare against analytical solution."""
        times = [0.0025, 0.005, 0.01]  # seconds
        for time in times:
            moose_coords, moose_temperature = read_moose_exodus_data(RUN_DIR / "run_out.e",
                                                                time,
                                                                "T")
            foam_coords, foam_temperature = read_openfoam_data(FOAM_CASE,
                                                          time,
                                                          'T')
            x = np.concatenate([moose_coords['x'], foam_coords['x']])
            temp = np.concatenate([moose_temperature, foam_temperature])

            analytic_temp = unsteady1d_temp(
                x=x,
                time=time,
                temp_cold=T_COLD,
                temp_hot=T_HOT,
                k1=K_SOLID,
                k2=K_FLUID,
                rho_cp1=RHO_CP_SOLID,
                rho_cp2=RHO_CP_FLUID,
            )

            rmse = np.sqrt(np.sum(np.square(analytic_temp - temp)) / len(temp))
            self.assertLess(rmse, 5e-3, msg=f"for time = {time} s")


def moose_get_temp_x(exo_file: Path,
                     time: float | int)-> tuple[np.ndarray, np.ndarray]:
    """Get x coordinate and temperature on a line though the solid.

    Parameters
    ----------
    exo_file : Path
        Exodus file from MOOSE
    time : float | int
        time

    Returns
    -------
    np.ndarray
        x coordinates of line
    np.ndarray
        temperature on line
    """
    moose_reader = pv.get_reader(exo_file)
    moose_reader.set_active_time_value(time)
    moose_data = moose_reader.read()[0].combine(True)
    moose_line = moose_data.sample_over_line((-1,0,0), (0,0,0), 100)
    return moose_line.points[:,0], moose_line['T']

def foam_get_temp_x(foam_dir,
                     time):
    """Get x coordinate and temperature on a line though the fluid.

    Parameters
    ----------
    exo_file : Path
        OpenFOAM directory
    time : float | int
        time

    Returns
    -------
    np.ndarray
        x coordinates of line
    np.ndarray
        temperature on line
    """

    file_name = Path(foam_dir) / "case.foam"
    file_name.touch()

    reader: pv.POpenFOAMReader = pv.get_reader(file_name)
    reader.set_active_time_value(time)
    foam_data: pv.UnstructuredGrid = reader.read()['internalMesh']

    foam_line = foam_data.sample_over_line((0,0,0), (1,0,0), 100)

    return foam_line.points[:,0], foam_line['T']

def main():
    """Main function for plotting output of 1D test."""

    # Here we leave some helpful debugging code that generates figures
    # to check this by eye.
    # We also perform a fit over the measured temperatures to check we
    # get the expected thermal properties (conductivity, rho*Cp) back
    # out.

    def find_nearest(array, value):
        idx = np.searchsorted(array, value, side="left")
        if idx > 0 and (
            idx == len(array)
            or np.fabs(value - array[idx - 1]) < np.fabs(value - array[idx])
        ):
            return idx - 1
        else:
            return idx

    _, axs = plt.subplot_mosaic([["1", "2"], ["3", "3"]])
    for i, time in enumerate([0.0025, 0.005, 0.01]):

        moose_x, moose_temperature = moose_get_temp_x(RUN_DIR / "run_out.e", time)
        foam_x, foam_temperature = foam_get_temp_x(FOAM_CASE, time)
        # Sample down the OpenFOAM values so the plot is less busy
        dx = foam_x[-1] - foam_x[-2]
        foam_x2 = [foam_x[0]]
        foam_temperature2 = [foam_temperature[0]]
        for _ in range(1, len(foam_x)):
            next_value = foam_x2[-1] + dx
            dx *= 1.1
            idx = find_nearest(foam_x, next_value)
            if np.isclose(foam_x[idx], foam_x2[-1]):
                continue
            foam_x2.append(foam_x[idx])
            foam_temperature2.append(foam_temperature[idx])

        x_space = np.linspace(-L, L, 201)
        analytic_temp = unsteady1d_temp(
            x_space,
            time,
            T_COLD,
            T_HOT,
            K_SOLID,
            K_FLUID,
            RHO_CP_SOLID,
            RHO_CP_FLUID,
        )

        # Best fit
        fit_x = np.concatenate([moose_x, foam_x])
        fit_y = np.concatenate([moose_temperature, foam_temperature])
        fit_kwargs = {"time": time, "temp_cold": T_COLD, "temp_hot": T_HOT}
        popt, pcov = curve_fit(
            lambda x, k1, k2, rho_cp1, rho_cp2: unsteady1d_temp(
                x, k1=k1, k2=k2, rho_cp1=rho_cp1, rho_cp2=rho_cp2, **fit_kwargs
            ),
            fit_x,
            fit_y,
            p0=[K_SOLID, K_FLUID, RHO_CP_SOLID, RHO_CP_FLUID],
            bounds=([0, 0, 0, 0], [np.inf, np.inf, np.inf, np.inf]),
        )
        print(f"{popt=}")

        # Calculate RMS
        x = np.concatenate([moose_x, foam_x])
        y = np.concatenate([moose_temperature, foam_temperature])
        expected_y = unsteady1d_temp(x, time, T_COLD, T_HOT, 1, 4, 1, 16)
        rmse = np.sqrt(np.sum(np.square(y - expected_y)) / len(y))
        print(f"{time}s RMS error: {rmse:e}")

        ax = axs[str(i + 1)]
        ax.scatter(moose_x, moose_temperature, label="MOOSE", marker="v", color="brown")
        ax.scatter(
            foam_x2,
            foam_temperature2,
            label="OpenFOAM",
            marker="o",
            s=11,
            edgecolors="blue",
            facecolors="none",
            linewidth=1,
        )
        ax.plot(x_space, analytic_temp, "--", label="Analytic")
        ax.plot(
            x_space,
            unsteady1d_temp(x_space, time, T_COLD, T_HOT, *popt),
            "r-.",
            label="fit",
            linewidth=0.5,
        )
        ax.set_title(f"Time = {time} s")
        ax.set_xlabel(r"$x$ $[m]$")
        ax.set_ylabel(r"Temperature $[K]$")
        ax.legend()
    plt.show()


if __name__ == "__main__":
    main()
