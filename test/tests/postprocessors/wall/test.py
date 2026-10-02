"""Analytical wall averages and integrals on the graded side-average fixture."""

import unittest

import numpy as np


class TestWallPostprocessors(unittest.TestCase):
    def test_wall_quantities(self):
        data = np.genfromtxt("main_out.csv", delimiter=",", names=True)
        data = data[data["time"] > 0]
        time = data["time"]
        np.testing.assert_allclose(time, np.arange(1, 33) * 0.01, rtol=0, atol=1e-14)

        # T=x*t, kappa=1; end areas are 2, top area is 20.
        expected = {
            "temperature_right": 10 * time,
            "temperature_left": 0 * time,
            "temperature_top": 5 * time,
            "temperature_combined": 60 * time / 11,
            "temperature_opposite": 5 * time,
            "flux_right": time,
            "flux_left": -time,
            "flux_top": 0 * time,
            "flux_combined": time / 11,
            "flux_opposite": 0 * time,
            "integral_right": 2 * time,
            "integral_left": -2 * time,
            "integral_top": 0 * time,
            "integral_combined": 2 * time,
            "integral_opposite": 0 * time,
            "htc_right": time / (10 * time + 2),
            "htc_left": -time,
            "htc_opposite": (time / (10 * time + 2) - time) / 2,
            "htc_combined": time / (10 * time + 2) / 11,
        }

        # Compare against expected value
        for name, values in expected.items():
            with self.subTest(postprocessor=name):
                np.testing.assert_allclose(data[name], values, rtol=1e-12, atol=1e-14)
