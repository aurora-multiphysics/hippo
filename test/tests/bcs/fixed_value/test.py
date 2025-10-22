"""Tests for imposing BCs in OpenFOAM using MOOSE input file syntax
"""

import unittest
import fluidfoam as ff
import numpy as np

from read_hippo_data import get_foam_times #pylint: disable=E0401

class TestFoamBCFixedValue(unittest.TestCase):
    """Test class for imposing fixed value BCs in Hippo."""
    def test_fixed_value(self):
        """Test case for imposing fixed value."""
        case_dir = 'foam/'
        boundaries = ['top', 'bottom', 'front', 'back', 'left', 'right']
        times = get_foam_times(case_dir)[1:]

        for time in times:
            for boundary in boundaries:
                coords = dict(zip(('x','y','z'),ff.readof.readmesh(case_dir, boundary=boundary)))

                temp = ff.readof.readscalar(case_dir, time, "T", boundary=boundary)

                scale = 1. if boundary in ('left', 'right', 'top') else 2.
                temp_ref = 0.05 + scale*(coords['x'] + coords['y'] + coords['z'])*np.float64(time)

                temp_diff_max = np.argmax(abs(temp-temp_ref))
                assert np.allclose(temp_ref, temp, rtol=1e-7, atol=1e-12),\
                            (f"Max diff {boundary} ({time}): {abs(temp-temp_ref).max()} "
                             f"{temp[temp_diff_max]} {temp_ref[temp_diff_max]}")
