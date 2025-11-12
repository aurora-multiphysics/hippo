
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


class TestCrankNicolsonSolver(TestCase):
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
        Crank Nicolson scheme can solve the ODE to machine precision. However, the first
        step uses backward Euler, so its truncation error must be propgated
        The truncation error for the backward Euler scheme here is $h^2 T''/2 = 500h^2$
        for the first time step.

        The expected numerical solution is hence 500(t^2 + h^2)
        """

        case_dir = 'foam/'

        times = get_foam_times(case_dir)
        h = 0.0003125
        for time in times:
            # internal data
            T = ff.readof.readscalar(case_dir, f"{time:g}", "T")
            exact = 500*(time*time + h*h) if time > 1e-5 else 0.
            assert np.allclose(T, exact, rtol=1e-12, atol=1e-12), f"Max diff ({time}): {abs(T-exact).max()}"
