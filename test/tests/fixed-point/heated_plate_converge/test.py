"""Regression test of fixed-point flow over heated plate problem"""
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


class TestFlowOverHeatedPlate(TestCase):
    """Compares the flow over heated plate problem with fixed-point iteration to a refernce case

    Details
    ------------
    This is the same problem as the multiapp test, but fixed-point iteration is used to
    improve convergence. The final result is compared to a reference case
    """

    def test_times(self):
        """Compares fixed-point solution against reference case."""
        case_dir = 'fluid-openfoam/'
        ref_dir = 'gold/'

        boundaries = ['inlet', 'outlet', 'top', 'slip-bottom', 'bottom', 'interface']

        times = get_foam_times(case_dir, True)
        for time in times:
            # internal data
            temp = ff.readof.readscalar(case_dir, time, "T")
            temp_ref = ff.readof.readscalar(ref_dir, time, "T")
            assert np.allclose(temp_ref, temp, rtol=1e-10), f"Max diff ({time}): {abs(temp-temp_ref).max()}"

            # boundary data
            for boundary in boundaries:
                temp = ff.readof.readscalar(case_dir, time, "T",
                                         boundary=boundary)
                temp_ref = ff.readof.readscalar(ref_dir, time, "T",
                                             boundary=boundary)
                assert np.allclose(temp_ref, temp, rtol=1e-10), f"Max diff ({time}): {abs(temp-temp_ref).max()}"
