"""Check halfway checkpointing and recovery against the analytical temperature field."""

import unittest

import fluidfoam as ff
import numpy as np


class TestDirectBC(unittest.TestCase):
    """Check temperatures before and after checkpoint recovery.

    INITIAL-only transfers prescribe T = 100 + x + y + z throughout.
    The run stops at time two and recovers to time four without retransferring
    the BC inputs. Compare output at times two, three and four with the
    analytical solution to check that the prescribed BCs survive recovery.
    """

    def test_recovery(self):
        x, y, z = ff.readmesh("child_case", precision=17, verbose=False)
        expected = 100 + x + y + z
        # Time two precedes recovery, times three and four follow it.
        for time in (2, 3, 4):
            with self.subTest(time=time):
                recovered = ff.readscalar("child_case", str(time), "T", verbose=False)
                np.testing.assert_allclose(recovered, expected, rtol=1e-7, atol=1e-12)
