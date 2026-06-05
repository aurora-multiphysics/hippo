"""Test module for the side average postprocessor."""

import unittest

import numpy as np

U = [2, -1, 0]
RHO = 0.5
AREAS = [1.0, 10.0, 10.0]
T_AVG = [0, 5, 10.0]

INDICES = [
    "U_avg_magnitude",
    "U_avg_magnitude_multiple",
    "U_avg_normal",
    "U_avg_normal_multiple",
    "U_avg_x",
    "U_avg_x_multiple",
    "U_avg_y",
    "U_avg_y_multiple",
    "U_avg_z",
    "heat_flux",
    "heat_flux_multiple",
    "t_avg",
    "t_avg_multiple",
]


class TestSideAverage(unittest.TestCase):
    """Test class for side average postprocessor."""

    def test_side_average(self):
        """Compares output csv to expected values for dfferent combinations."""
        data = np.loadtxt("main_out.csv", delimiter=",", skiprows=2, unpack=True)
        time = data[0]
        data = dict(zip(INDICES, data[1:]))

        tol_params = {"atol": 1e-14, "rtol": 1e-12}

        assert np.allclose(data["t_avg"], T_AVG[1] * time, **tol_params)
        assert np.allclose(data["t_avg_multiple"], T_AVG[1] * time, **tol_params)
        assert np.allclose(data["U_avg_magnitude"], np.linalg.norm(U), **tol_params)
        assert np.allclose(
            data["U_avg_magnitude_multiple"], np.linalg.norm(U), **tol_params
        )
        assert np.allclose(data["U_avg_normal"], U[0], **tol_params)
        assert np.allclose(data["U_avg_normal_multiple"], 0.0, **tol_params)
        assert np.allclose(data["U_avg_x"], U[0], **tol_params)
        assert np.allclose(data["U_avg_x_multiple"], U[0], **tol_params)
        assert np.allclose(data["U_avg_y"], U[1], **tol_params)
        assert np.allclose(data["U_avg_y_multiple"], U[1], **tol_params)
        assert np.allclose(data["U_avg_z"], U[2], **tol_params)
        assert np.allclose(data["heat_flux"], time, **tol_params)
        assert np.allclose(data["heat_flux_multiple"], 0.0, **tol_params)
