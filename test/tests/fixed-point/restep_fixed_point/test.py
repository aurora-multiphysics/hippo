from unittest import TestCase

import fluidfoam as ff
import numpy as np

from read_hippo_data import get_foam_times


class TestUnsteadyFixedPointConverge(TestCase):
    """Regression test of the 1D unsteady heat conduction problem using fixed-point to improve convergence."""

    def test_times(self):
        """Compares against reference converged solution."""
        case_dir = "foam/"
        ref_dir = "gold/"

        boundaries = ["left", "right", "top", "bottom", "front", "back"]

        times = get_foam_times(case_dir, True)
        for time in times:
            # internal data
            T = ff.readof.readscalar(case_dir, time, "T")
            T_ref = ff.readof.readscalar(ref_dir, time, "T")
            assert np.array_equal(T_ref, T), f"Max diff: {abs(T - T_ref).max()}"

            # boundary data
            for boundary in boundaries:
                T = ff.readof.readscalar(case_dir, time, "T", boundary=boundary)
                T_ref = ff.readof.readscalar(ref_dir, time, "T", boundary=boundary)
                assert np.array_equal(T_ref, T), f"Max diff: {abs(T - T_ref).max()}"
