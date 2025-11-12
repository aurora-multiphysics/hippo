
import os
import re
from unittest import TestCase

import fluidfoam as ff
import numpy as np


def get_foam_times(case_dir: str | bytes, string=False) -> list[str]:
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
    return sorted(folder if string else np.float64(folder)
                  for folder in os.listdir(case_dir)
                  if re.match(r"[-+]?([0-9]*\.[0-9]+|[0-9]+)", folder))


class TestUnsteadyFixedPointConverge(TestCase):
    """Regression test of the 1D unsteady heat conduction problem using fixed-point to improve convergence."""

    def test_times(self):
        """Compares fixed-point solution against non-fixed-point solution."""
        case_dir = 'foam/'
        ref_dir = 'gold/'

        boundaries = ['left', 'right', 'top', 'bottom', 'front', 'back']

        times = get_foam_times(case_dir, True)
        for time in times:
            # internal data
            T = ff.readof.readscalar(case_dir, time, "T")
            T_ref = ff.readof.readscalar(ref_dir, time, "T")
            assert np.array_equal(T_ref, T), f"Max diff: {abs(T-T_ref).max()}"

            # boundary data
            for boundary in boundaries:
                T = ff.readof.readscalar(case_dir, time, "T",
                                         boundary=boundary)
                T_ref = ff.readof.readscalar(ref_dir, time, "T",
                                             boundary=boundary)
                assert np.array_equal(T_ref, T), f"Max diff: {abs(T-T_ref).max()}"
