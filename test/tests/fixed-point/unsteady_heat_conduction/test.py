"""Test module for the unsteady 1D heat conduction problem"""

import unittest
from pathlib import Path

import fluidfoam as ff
import numpy as np
import pyvista as pv

# import Hippo test python functions
from read_hippo_data import get_foam_times, read_moose_exodus_data

RUN_DIR = Path(__file__).parent


class TestUnsteadyHeatConductionInInfiniteSystem(unittest.TestCase):
    """Test class for 1D unsteady heat conduction problem"""

    def test_solid_fixed_point(self):
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
        times = get_foam_times("foam")
        for time in times:
            temp = ff.readof.readscalar("foam", f"{time:g}", "T", verbose=False)

            temp_ref = ff.readof.readscalar("gold", f"{time:g}", "T", verbose=False)

            assert np.array_equal(temp, temp_ref), (
                f"Max diff ({time}): {abs(temp - temp_ref).max()}"
            )
