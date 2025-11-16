"""Test module for the unsteady 1D heat conduction problem"""

import unittest
import os
import re

from pathlib import Path

import numpy as np
import fluidfoam as ff
import pyvista as pv

# import Hippo test python functions
from read_hippo_data import read_moose_exodus_data

RUN_DIR = Path(__file__).parent
FOAM_CASE = RUN_DIR / "foam"
K_SOLID = 1
RHO_CP_SOLID = 1
K_FLUID = 4
RHO_CP_FLUID = 16
T_HOT = 1.
T_COLD = 0.
L = 1


def get_foam_times(case_dir: str | bytes, use_float=True) -> list[str]:
    """Get the times from the foam case directory

    Parameters
    ----------
    case_dir : str | bytes
        Case directory

    Returns
    -------
    list[str]
        Sorted list of times
    """
    return sorted(np.float64(folder) if use_float else folder
                  for folder in os.listdir(case_dir)
                  if re.match(r"[-+]?([0-9]*\.[0-9]+|[0-9]+)", folder))


class TestUnsteadyHeatConductionInInfiniteSystem(unittest.TestCase):
    """Test class for 1D unsteady heat conduction problem"""

    def test_solid_fixed_point(self):
        """Compare solid temperature to reference without fixed-point"""
        reader = pv.get_reader("main_out.e")
        times = reader.time_values

        for time in times:
            _, solid_temp = read_moose_exodus_data(RUN_DIR / "main_out.e",
                                                   time,
                                                   "T")
            _, solid_temp_ref = read_moose_exodus_data(RUN_DIR / "gold" / "main_out.e",
                                                       time,
                                                       "T")

            assert np.array_equal(
                solid_temp, solid_temp_ref),(f"Max diff ({time}): "
                                             f"{abs(solid_temp-solid_temp_ref).max()}")

    def test_fluid_fixed_point(self):
        """Compare fluid temperature to reference without fixed-point"""

        times = get_foam_times("foam", False)
        for time in times:
            temp = ff.readof.readscalar("foam", time, "T", verbose=False)

            temp_ref = ff.readof.readscalar("gold", time, "T", verbose=False)

            assert np.array_equal(
                temp, temp_ref), f"Max diff ({time}): {abs(temp-temp_ref).max()}"
