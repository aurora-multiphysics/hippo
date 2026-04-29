"""test module for the side advective flux integral postprocessor."""

import unittest

import numpy as np


U = [2, -1, 0]
RHO = 0.5
AREAS = [1.0, 10.0, 10.0]


class TestSideAdvectiveFluxIntegral(unittest.TestCase):
    """Test class for the side advective flux integral postprocessor."""

    def test_advective_flux_integral(self):
        """Tests the output csv matches the expected value."""
        data = np.loadtxt("main_out.csv", delimiter=",", skiprows=2, unpack=True)[1:]

        # loop over x, y, z surface postprocessors
        for i in range(3):
            # n indicates the outward normal relative to the coordinate direction
            n = -1.0
            assert np.allclose(
                data[i * 3], n * RHO * U[i] * AREAS[i], atol=1e-14, rtol=1e-12
            ), f"{data[i * 3]} vs {n * RHO * U[i] * AREAS[i]}"

            assert np.allclose(data[i * 3 + 1], 0.0, atol=1e-14, rtol=1e-12)

            n = 1.0
            assert np.allclose(
                data[i * 3 + 2], n * RHO * U[i] * AREAS[i], atol=1e-14, rtol=1e-12
            )
