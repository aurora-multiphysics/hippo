"""Tests for imposing BCs in OpenFOAM using MOOSE input file syntax"""

import unittest

import fluidfoam as ff
import numpy as np


class TestFoamBCFixedGradient(unittest.TestCase):
    """Test class for imposing fixed value BCs in Hippo."""

    def test_diffusion_flux_x(self):
        """
        Test case for imposing diffusion flux BCs using the default value of the
        underlying reciever.

        Solves laplace equation with right BC being \partial_x T = 1.
        The analytical solution is x.
        """
        case_dir = "foam/"
        time = "1"
        coords = dict(zip(("x", "y", "z"), ff.readof.readmesh(case_dir)))

        temp = ff.readof.readscalar(case_dir, time, "T")

        temp_ref = coords["x"]
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
