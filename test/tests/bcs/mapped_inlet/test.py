"""Tests for imposing BCs in OpenFOAM using MOOSE input file syntax
"""

import unittest
import fluidfoam as ff
import numpy as np

from read_hippo_data import get_foam_times

CASE_DIR = 'foam/'
TIMES = get_foam_times(CASE_DIR)[1:]


class TestFoamBCMappedInlet(unittest.TestCase):
    """Test class for mapped inlet BCs in Hippo."""
    def test_mapped_inlet(self):
        """Test case for mapped inlet."""

        for i in range(len(TIMES)):
            self._check_u_temp_refs(i, 'left', [1., 0, 0])
            self._check_u_temp_refs(i, 'bottom', [0, 1., 0])
            self._check_u_temp_refs(i, 'front', [0, 0, 1.])

    def test_mapped_inlet_subtract(self):
        """Test case for mapped inlet when temperature is scaled by subtracting the difference in bulk."""

        for i in range(len(TIMES)):
            self._check_u_temp_refs(i, 'left', [1., 0, 0], False)
            self._check_u_temp_refs(i, 'bottom', [0, 1., 0], False)
            self._check_u_temp_refs(i, 'front', [0, 0, 1.], False)

    def test_mapped_inlet_rotated(self):
        """Test case for when inlet's are not aligned with the axis."""
        for i in range(len(TIMES)):
            self._check_u_temp_refs(i, 'left', [np.sqrt(0.5), np.sqrt(0.5), 0])

    def test_mapped_inlet_face_point(self):
        """Test case for mapped inlet where the point is on the interface between boundaries."""

        for i in range(len(TIMES)):
            self._check_u_temp_refs(i, 'left', [7./8, 0, 0])
            self._check_u_temp_refs(i, 'bottom', [0, 7./8, 0])
            self._check_u_temp_refs(i, 'front', [0, 0, 7./8])

    def _check_u_temp_refs(self, idx, boundary, offset, use_scale=True):
        rho = 0.5
        mdot_pp = 1
        t_pp = 1
        time = TIMES[idx]

        x, y, z = ff.readof.readmesh(CASE_DIR, boundary=boundary)
        u = ff.readof.readvector(CASE_DIR, time, "U", boundary=boundary).T
        temp = ff.readof.readscalar(CASE_DIR, time, "T", boundary=boundary)

        x += offset[0]
        y += offset[1]
        z += offset[2]

        if idx != 0:
            t = np.float64(TIMES[idx-1])
            u_ref = np.array([x + y + z + t, x - y + z + t, x + y - z + t,]).T
            temp_ref = np.sqrt(x*x + y*y + z*z) + t
        else:
            # first time step uses initialised value
            u_ref = np.array([[1, -0.5, 0.25]])
            temp_ref = 2

        area = 4.
        normal = np.array(offset)/np.linalg.norm(offset)
        mdot = rho*np.mean(np.vecdot(u_ref, normal))*area
        u_ref *= mdot_pp/mdot

        t_bulk = np.mean(temp_ref)
        if use_scale:
            temp_ref *= t_pp/t_bulk
        else:
            temp_ref += (t_pp - t_bulk)


        assert np.allclose(u_ref, u, rtol=1e-7, atol=1e-12),\
                    f"Max diff ({boundary}) (velocity) ({TIMES[idx]}): {abs(u-u_ref).max()} "

        assert np.allclose(temp_ref, temp, rtol=1e-7, atol=1e-12),\
                    f"Max diff ({boundary}) (temperature) ({time}): {abs(temp-temp_ref).max()} {temp} {temp_ref}"
