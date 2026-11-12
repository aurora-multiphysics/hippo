"""Independent per-face checks of native output, without a mirrored reference."""

from pathlib import Path
import unittest

import fluidfoam as ff
import numpy as np


class TestTransfer(unittest.TestCase):
    """Check native output from the serial and MPI spatial transfer runs."""

    def read_values(self, case, time, boundary=None):
        self.assertTrue((Path(case) / str(time) / "transferField").is_file())
        values = np.asarray(
            ff.readscalar(
                case,
                time_name=str(time),
                name="transferField",
                boundary=boundary,
                precision=17,
                verbose=False,
            ),
            dtype=np.float64,
        ).reshape(-1)
        # OpenFOAM may write a uniform value instead of an explicit list.
        self.assertTrue(np.isfinite(values).all())
        return values

    def test_spatial(self):
        self.check_target_boundaries(
            lambda x, y, z, t: 10 + 2 * x + 3 * y + 5 * z + t * (1 + x + 2 * y + 3 * z)
        )

    def check_target_boundaries(self, expected_value):
        for boundary in ("left", "right"):
            x, y, z = ff.readmesh(
                "child_case", boundary=boundary, precision=17, verbose=False
            )
            self.assertEqual(len(x), 6)
            for time in (1, 2, 3):
                with self.subTest(boundary=boundary, time=time):
                    actual = self.read_values("child_case", time, boundary)
                    actual = actual[0] if actual.size == 1 else actual
                    expected = expected_value(x, y, z, time)
                    np.testing.assert_allclose(actual, expected, atol=1e-12, rtol=0)
