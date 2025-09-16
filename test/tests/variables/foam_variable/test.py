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

            T_ref = 0.01 + (coords['x']*coords['y'] + coords['z']*coords['y'] + coords['z']*coords['x'])*time
            T_diff_max = np.argmax(abs(T-T_ref))
            assert np.allclose(T_ref, T, rtol=1e-7, atol=1e-12),\
                        f"Max diff ({time}): {abs(T-T_ref).max()} {T[T_diff_max]} {T_ref[T_diff_max]}"

    def test_wall_heat_flux_transfer(self):

        times = get_exodus_times("main_out.e")[1:]
        for time in times:
            coords, whf = read_moose_exodus_data('main_out.e',
                                                 time,
                                                 variable="whf_shadow")

            sum_ = 0
            whf_ref = np.zeros_like(whf)
            for comp in ('x', 'y', 'z'):
                low = coords[comp]<coords[comp].min()+1e-6
                high = coords[comp]>(coords[comp].max()-1e-6)
                sum_ += np.sum(low) + np.sum(high)
                c = ('y', 'z') if comp == 'x' else ('z', 'x') if comp == 'y' else ('x', 'y')
                whf_ref[low] = -time*(coords[c[0]][low] + coords[c[1]][low])
                whf_ref[high] = time*(coords[c[0]][high] + coords[c[1]][high])

            assert sum_ == whf.size, f"{sum_} {whf.size}"

            T_diff_max = np.argmax(abs(whf-whf_ref))
            assert np.allclose(whf_ref, whf, rtol=1e-7, atol=1e-12),\
                        f"Max diff ({time}): {abs(whf-whf_ref).max()} {whf[T_diff_max]}"
