"""Tests for imposing BCs in OpenFOAM using MOOSE input file syntax
"""

import unittest
import fluidfoam as ff
import numpy as np

from read_hippo_data import get_foam_times

class TestFoamBCFixedGradient(unittest.TestCase):
    """Test class for imposing fixed gradient BCs in Hippo."""
    def test_fixed_gradient_x(self):
        """Test case for imposing fixed gradient."""
        case_dir = 'foam/'
        times = get_foam_times(case_dir)[1:]

        for time in times:
            coords = dict(zip(('x','y','z'),ff.readof.readmesh(case_dir)))

            temp = ff.readof.readscalar(case_dir, time, "T")

            temp_ref = coords['x']*np.float64(time)

            temp_diff_max = np.argmax(abs(temp-temp_ref))
            assert np.allclose(temp_ref, temp, rtol=1e-7, atol=1e-12),\
                        (f"Max diff ({time}): {abs(temp-temp_ref).max()} "
                            f"{temp[temp_diff_max]} {temp_ref[temp_diff_max]}")
