"""Test module for the heat transfer coefficient foam variable."""

import unittest

from read_hippo_data import get_exodus_times, read_moose_exodus_data


class TestFoamHeatTransferCoeff(unittest.TestCase):
    """Test class for FoamHeatTransferCoeff."""

    def test_htc(self):
        """
        Compares output exodus output against expected solution.
        Temperature field is set to x * t.
        q_w = kappa*t, with kappa = 0.5
        h = q_w * (T_w - T_b), with T_b = 1
        h = 0.5*t
        """

        times = get_exodus_times("main_out.e")
        for t in times:
            _, htc = read_moose_exodus_data("main_out.e", t, "htc")

            assert abs(htc[0] - 0.5 * t) < 1e-12, (
                f"HTC wrong at time {t}: {htc[0]} vs {0.5 * t}"
            )
