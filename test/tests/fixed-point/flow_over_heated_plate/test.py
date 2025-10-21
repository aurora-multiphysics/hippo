
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
    """Compares the flow over heated plate problem with fixed-point iteration to one without

    Details
    ------------
    This is the same problem as the multiapp test, but the fixed-point solution only updates
    the boundary condition at the start of the fixed-point loop so that the fixed-point solution
    should be the same as one without. This test checks for an exact match for the temperature
    field at all times.
    """

    def test_times(self):
        """Compares fixed-point solution against non-fixed-point solution."""
        case_dir = 'fluid-openfoam/'
        ref_dir = 'gold/'

        boundaries = ['inlet', 'outlet', 'top', 'slip-bottom', 'bottom', 'interface']

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
