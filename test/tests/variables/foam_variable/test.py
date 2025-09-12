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

            T_ref = 0.01 + coords['x']*coords['y']*coords['z']*time
            T_diff_max = np.argmax(abs(T-T_ref))
            assert np.allclose(T_ref, T, rtol=1e-7, atol=1e-12), f"Max diff ({time}): {abs(T-T_ref).max()} {T[T_diff_max]}"
