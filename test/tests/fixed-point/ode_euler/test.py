
import os
import re
from unittest import TestCase

import fluidfoam as ff
import numpy as np


def get_foam_times(case_dir: str | bytes) -> list[str]:
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
    return sorted(np.float64(folder)
                  for folder in os.listdir(case_dir)
                  if re.match(r"[-+]?([0-9]*\.[0-9]+|[0-9]+)", folder))


class TestEulerSolver(TestCase):
    """Tests fixed-point against an ODE with an analytical solution.

    Details
    ------------
    Solves $\frac{dT}{dt} = 1000t$, comparing the fixed-point solution against
    non-fixed-point and analytical
    """

    def test_times(self):
        """Compares fixed-point solution against non-fixed-point solution."""
        case_dir = 'foam/'
        ref_dir = 'gold/'

        times = get_foam_times(case_dir)
        for time in times:
            # internal data
            T = ff.readof.readscalar(case_dir, f"{time:g}", "T")
            T_ref = ff.readof.readscalar(ref_dir, f"{time:g}", "T")
            assert np.array_equal(T_ref, T), f"Max diff: {abs(T-T_ref).max()}"

    def test_analytical(self):
        """Compare ODE solution with analytical solution

        The analytical solution is T = 500t^2
        For the backward Euler scheme the ODE cannot be solved to machine precision.
        The truncation error for the backward Euler scheme here is $h^2 T''/2 = 500h^2$
        for each time step, leading to an error, $\varepsilon(t) = 500t h/2$

        The expected numerical solution is hence 500t(t+h)
        """

        case_dir = 'foam/'

        times = get_foam_times(case_dir)
        h = 0.0003125
        for time in times:
            # internal data
            T = ff.readof.readscalar(case_dir, f"{time:g}", "T")
            exact = 500*time*(time + h)
            assert np.allclose(T, exact, rtol=1e-12, atol=1e-12), f"Max diff ({time}): {abs(T-exact).max()}"
