from unittest import TestCase

import fluidfoam as ff
import numpy as np

from read_hippo_data import get_foam_times


class TestLaplacianSolver(TestCase):
    """Compares the solution of a laplacian with fixed-point iteration to one without

    Details
    ------------
    Solves $\nabla T^n+1 - T^n = 0$ and so includes current and old times. The left BC is
    given by the current time. This is a steady problem whose solution varies with time
    due time-dependent BCs and an equation that depends on the previous solution. This
    test case compares against one where fixed-point isn't used, the results should be
    the same.
    """

    def test_times(self):
        """Compares fixed-point solution against non-fixed-point solution."""
        case_dir = "foam/"
        ref_dir = "gold/"

        boundaries = ["left", "right", "bottom", "top", "back", "front"]

        times = get_foam_times(case_dir)
        for time in times:
            # internal data
            T = ff.readof.readscalar(case_dir, f"{time:g}", "T")
            T_ref = ff.readof.readscalar(ref_dir, f"{time:g}", "T")
            assert np.array_equal(T_ref, T), f"Max diff: {abs(T - T_ref).max()}"

            # boundary data
            for boundary in boundaries:
                T = ff.readof.readscalar(case_dir, f"{time:g}", "T", boundary=boundary)
                T_ref = ff.readof.readscalar(
                    ref_dir, f"{time:g}", "T", boundary=boundary
                )
                assert np.array_equal(T_ref, T), f"Max diff: {abs(T - T_ref).max()}"
