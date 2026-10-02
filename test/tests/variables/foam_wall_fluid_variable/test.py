"""Analytical temperature and heat flux transferred through fluid wall variables"""

import unittest

import numpy as np
from read_hippo_data import (  # pylint: disable=E0401
    get_exodus_times,
    read_moose_exodus_data,
)


class TestFoamVariableFluid(unittest.TestCase):
    """Test class for shadowing scalar fields in Hippo."""

    def test_wall_temperature(self):
        """Test case for shadowing a temperature."""

        all_times = get_exodus_times("main_out.e")
        np.testing.assert_allclose(
            all_times, np.arange(33) * 0.01, rtol=1e-7, atol=1e-12
        )
        times = all_times[1:]
        for time in times:
            coords, temp = read_moose_exodus_data(
                "main_out.e", time, variable="T_shadow"
            )

            temp_ref = (
                0.01
                + (
                    coords["x"] * coords["y"]
                    + coords["z"] * coords["y"]
                    + coords["z"] * coords["x"]
                )
                * time
            )
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

    def test_wall_heat_flux(self):
        """Check the signed normal conductive heat flux on every patch."""
        all_times = get_exodus_times("main_out.e")
        np.testing.assert_allclose(
            all_times, np.arange(33) * 0.01, rtol=1e-7, atol=1e-12
        )
        times = all_times[1:]
        for time in times:
            coords, whf = read_moose_exodus_data(
                "main_out.e", time, variable="whf_shadow"
            )

            sum_ = 0
            whf_ref = np.zeros_like(whf)
            for comp in ("x", "y", "z"):
                low = coords[comp] < coords[comp].min() + 1e-6
                high = coords[comp] > (coords[comp].max() - 1e-6)
                sum_ += np.sum(low) + np.sum(high)
                c = (
                    ("y", "z")
                    if comp == "x"
                    else ("z", "x")
                    if comp == "y"
                    else ("x", "y")
                )
                whf_ref[low] = -time * (coords[c[0]][low] + coords[c[1]][low])
                whf_ref[high] = time * (coords[c[0]][high] + coords[c[1]][high])

            assert sum_ == whf.size, f"{sum_} {whf.size}"

            whf_diff_max = np.argmax(abs(whf - whf_ref))
            np.testing.assert_allclose(
                whf_ref,
                whf,
                rtol=1e-7,
                atol=1e-12,
                err_msg=(
                    f"Max diff ({time}): {abs(whf - whf_ref)[whf_diff_max]} {whf[whf_diff_max]}"
                ),
            )
