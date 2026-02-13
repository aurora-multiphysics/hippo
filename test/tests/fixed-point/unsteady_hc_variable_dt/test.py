"""Test module for the unsteady 1D heat conduction problem"""

import unittest
from pathlib import Path

import fluidfoam as ff
import numpy as np
import pyvista as pv

# import Hippo test python functions
from read_hippo_data import get_foam_times, read_moose_exodus_data, read_openfoam_data
from analytical import unsteady1d_temp


RUN_DIR = Path(__file__).parent

K_SOLID = 1
RHO_CP_SOLID = 1
K_FLUID = 4
RHO_CP_FLUID = 16
T_HOT = 1.0
T_COLD = 0.0
L = 1


class TestUnsteadyHeatConductionInInfiniteSystem(unittest.TestCase):
    """Test class for 1D unsteady heat conduction problem"""

    def test_solid_fixed_point(self):
        """Compare solid temperature to reference without fixed-point"""
        reader = pv.get_reader("main_out.e")
        times = reader.time_values

        for time in times:
            _, solid_temp = read_moose_exodus_data(RUN_DIR / "main_out.e", time, "T")
            _, solid_temp_ref = read_moose_exodus_data(
                RUN_DIR / "gold" / "main_out.e", time, "T"
            )

            assert np.array_equal(solid_temp, solid_temp_ref), (
                f"Max diff ({time}): {abs(solid_temp - solid_temp_ref).max()}"
            )

    def test_fluid_fixed_point(self):
        """Compare fluid temperature to reference without fixed-point"""

        times = get_foam_times("foam", string=True)
        for time in times:
            temp = ff.readof.readscalar("foam", time, "T", verbose=False)

            temp_ref = ff.readof.readscalar("gold", time, "T", verbose=False)

            assert np.array_equal(temp, temp_ref), (
                f"Max diff ({time}): {abs(temp - temp_ref).max()}"
            )

    def test_analytical(self):
        """Compare against 1D unsteady analytical solution"""
        times = [0.0025, 0.005, 0.01]  # seconds
        for time in times:
            moose_coords, moose_temperature = read_moose_exodus_data(
                RUN_DIR / "main_out.e", time, "T"
            )
            foam_coords, foam_temperature = read_openfoam_data("foam", time, "T")
            x = np.concatenate([moose_coords["x"], foam_coords["x"]])
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
