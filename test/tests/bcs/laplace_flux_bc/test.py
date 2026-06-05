"""Tests for imposing BCs in OpenFOAM using MOOSE input file syntax"""

import unittest

import fluidfoam as ff
import numpy as np
from read_hippo_data import get_foam_times  # pylint: disable=E0401


class TestFoamBCDiffusionFlux(unittest.TestCase):
    """Test class for imposing flux BCs in Hippo."""

    def test_diffusion_flux_x(self):
        """
        Test case for imposing diffusion flux BCs.

        Solves laplace equation at each time step with right BC being f\partial_x T = 2t
        where k = 2.
        The analytical solution is t*x.
        """
        case_dir = "foam/"
        times = get_foam_times(case_dir, string=True)[1:]

        for time in times:
            coords = dict(zip(("x", "y", "z"), ff.readof.readmesh(case_dir)))

            temp = ff.readof.readscalar(case_dir, time, "T")

            temp_ref = coords["x"] * np.float64(time)

            temp_diff_max = np.argmax(abs(temp - temp_ref))
            np.testing.assert_allclose(
                temp_ref,
                temp,
                rtol=1e-7,
                atol=1e-12,
                err_msg=(
                    f"Max diff ({time}): {abs(temp - temp_ref).max()} "
                    f"{temp[temp_diff_max]} {temp_ref[temp_diff_max]}"
                ),
            )
