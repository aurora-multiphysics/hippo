
from unittest import TestCase
from pathlib import Path
import pyvista as pv
import fluidfoam as ff
from glob import glob
import re
import os
import numpy as np


def get_foam_times(case_dir: str | bytes):
    return sorted(np.float64(folder) for folder in os.listdir(case_dir) if re.match(r"[-+]?([0-9]*\.[0-9]+|[0-9]+)", folder))


class TestFunctionSolver(TestCase):
    """Test fixed-point iteration where the solver imposes the current time on the internal field.
    """

    def test_times(self):
        """
        Test that internal field equals the current time and that old field is the previous time.

        This check that the time is correctly set given that the times must be
        restored from stream
        """

        # read from openfoam file
        case_dir = 'foam/'
        time_dirs = get_foam_times(case_dir)[1:]
        for time in time_dirs:
            # set time and read internal field
            T = ff.readof.readscalar(
                case_dir, time_name=f"{time:g}", name='T', verbose=False)
            dTdt = ff.readof.readscalar(
                case_dir, time_name=f"{time:g}", name='dTdt', verbose=False)

            # check T array equals the time
            assert np.allclose(T, time,
                                   atol=0, rtol=1e-16), f"Max diff ({time}): {abs(T-time).max()}"

            ref = 1. if time > time_dirs[0] else 0.
            assert np.allclose(dTdt, ref,
                                   atol=0, rtol=1e-14), f"Max diff ({time}): {abs(dTdt-1).max()}"

    def test_compare_reference(self):
        case_dir = 'foam/'
        ref_dir = 'gold'
        time_dirs = get_foam_times(case_dir)[1:]

        for time in time_dirs:
            # set time and read internal field
            T = ff.readof.readscalar(
                case_dir, time_name=f"{time:g}", name='T', verbose=False)
            dTdt = ff.readof.readscalar(
                case_dir, time_name=f"{time:g}", name='dTdt', verbose=False)

            T_ref = ff.readof.readscalar(
                ref_dir, time_name=f"{time:g}", name='T', verbose=False)
            dTdt_ref = ff.readof.readscalar(
                ref_dir, time_name=f"{time:g}", name='dTdt', verbose=False)


            assert np.array_equal(T_ref, T), f"Max diff ({time}): {abs(T-T_ref).max()}"
            assert np.array_equal(dTdt, dTdt_ref), f"Max diff ({time}): {abs(dTdt-dTdt_ref).max()}"
