
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
        time_dirs = get_foam_times(case_dir)
        for i, time in enumerate(time_dirs):
            # set time and read internal field
            T = ff.readof.readscalar(
                case_dir, time_name=f"{time:g}", name='T', verbose=False)
            # check T array equals the time
            assert np.allclose(T, time,
                                   atol=0, rtol=1e-16), f"Max diff: {abs(T-time).max()}"

            # check that next `old time' is equal to the present time
            if i < len(time_dirs) - 1:
                T2 = ff.readof.readscalar(
                    case_dir, time_name=f"{time_dirs[i+1]:g}", name='T2', verbose=False)
                assert np.allclose(T2, time,
                                   atol=0, rtol=1e-16), f"Max diff: {abs(T2-time).max()}"
