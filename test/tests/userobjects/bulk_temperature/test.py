"""Test module for bulk temperature user objects"""

import unittest

import numpy as np


class TestAdjacentCellBulkTemperature(unittest.TestCase):
    """Class for adjacent cell bulk temperature user objects"""

    def test_adjacent_cell(self):
        """Compares output against expected solution"""
        times, temp = np.loadtxt(
            "adjacent_bulk_temperature_out.csv", unpack=True, skiprows=1, delimiter=","
        )

        for t, T in zip(times, temp):
            x0 = 0.5 * 10.0 / 50
            y0 = 0.5
            z0 = 0.5

            T_ref = 0.01 + (x0 * y0 + y0 * z0 + x0 * z0) * t
            assert abs(T_ref - T) < 1e-12, (
                f"Check temperatures match expected values {T_ref} vs {T}"
            )
