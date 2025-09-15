import unittest

import numpy as np

from read_hippo_data import read_moose_exodus_data, get_exodus_times

class TestFoamVariableTransfer(unittest.TestCase):
    def test_variable_transfer(self):

        times = get_exodus_times("main_out.e")[1:]
        for time in times:
            coords, T = read_moose_exodus_data('main_out.e',
                                                 time,
                                                 variable="T_shadow")

            T_ref = 0.01 + (coords['x'] + coords['y'] + coords['z'])*time
            T_diff_max = np.argmax(abs(T-T_ref))
            assert np.allclose(T_ref, T, rtol=1e-7, atol=1e-12),\
                        f"Max diff ({time}): {abs(T-T_ref).max()} {T[T_diff_max]} {T_ref[T_diff_max]}"

    def test_wall_heat_flux_transfer(self):

        times = get_exodus_times("main_out.e")[1:]
        for time in times:
            coords, whf = read_moose_exodus_data('main_out.e',
                                                 time,
                                                 variable="whf_shadow")

            eq = np.logical_or(np.logical_or(coords['x'] < 1e-6, coords['y'] < 1e-6),
                               coords['z'] < 1e-6)

            whf_ref = np.zeros_like(whf)
            whf_ref[eq] = - time
            whf_ref[~eq] = time


            T_diff_max = np.argmax(abs(whf-whf_ref))
            assert np.allclose(whf_ref, whf, rtol=1e-7, atol=1e-12),\
                        f"Max diff ({time}): {abs(whf-whf_ref).max()} {whf[T_diff_max]}"
