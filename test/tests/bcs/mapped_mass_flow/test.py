"""Tests for imposing BCs in OpenFOAM using MOOSE input file syntax
"""

import unittest
import fluidfoam as ff
import numpy as np

from read_hippo_data import get_foam_times

class TestFoamBCMappedInlet(unittest.TestCase):
    """Test class for mapped inlet BCs in Hippo."""
    def test_mapped_inlet(self):
        """Test case for mapped inlet."""
        case_dir = 'foam/'
        times = get_foam_times(case_dir)[1:]

        rho = 0.5
        for time in times:
            u = ff.readof.readvector(case_dir, time, "U", boundary='left')
            t = np.float64(time)

            if time != times[0]:
                x, y, z = ff.readof.readmesh(case_dir, boundary='left')
                x += 0.5
                u_ref = np.array([(x + y + z)*t, (x - y + z)*t, (x + y - z)*t,])
            else:
                # first time step uses initialised value
                u_ref = np.array([1, -0.5, 0.25])[:,None]

            rho = 0.5
            mdot = rho*np.mean(u_ref[0])
            mdot_pp = t
            u_ref *= mdot_pp/mdot

            assert np.allclose(u_ref, u, rtol=1e-7, atol=1e-12),\
                        f"Max diff ({time}): {abs(u-u_ref).max()} "
