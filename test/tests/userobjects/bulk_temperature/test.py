"""Test module for bulk temperature user objects"""

import unittest

import numpy as np


class TestAdjacentCellBulkTemperature(unittest.TestCase):
    """Class for adjacent cell bulk temperature user objects"""

    def test_adjacent_cell(self):
        """Compares output against expected solution"""
        times, temp1 = np.loadtxt(
            "main_out.csv", unpack=True, skiprows=1, delimiter=","
        )
        times, temp2 = np.loadtxt(
            "main_out_hippo0.csv", unpack=True, skiprows=1, delimiter=","
        )

        for t, T1, T2 in zip(times, temp1, temp2):
            # Expected adjacent cell expected at this location
            x0 = 0.5 * 10.0 / 50
            y0 = 0.5
            z0 = 0.5

            # Calculate T used in solver
            T_ref = 0.01 + (x0 * y0 + y0 * z0 + x0 * z0) * t
            assert abs(T_ref - T1) < 1e-12, (
                f"Check temperatures match expected values {T_ref} vs {T1}"
            )
            assert abs(T_ref - T2) < 1e-12, (
                f"Check temperatures match expected values {T_ref} vs {T2}"
            )
