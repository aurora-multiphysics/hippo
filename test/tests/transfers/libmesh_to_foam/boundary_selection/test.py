"""Check native transfer output against independently constructed source samples."""

import unittest

import fluidfoam as ff
import numpy as np


def source_value(points, time):
    """The function prescribed by main.i, evaluated in the parent frame."""
    x, y, z = np.moveaxis(np.asarray(points), -1, 0)
    return 10 + 2 * x + 3 * y + 5 * z + time * (1 + x + 2 * y + 3 * z)


class TestTransfer(unittest.TestCase):
    def read_values(self, time, boundary=None):
        values = np.asarray(
            ff.readscalar(
                "child_case",
                time_name=str(time),
                name="transferField",
                boundary=boundary,
                precision=17,
                verbose=False,
            ),
            dtype=np.float64,
        ).reshape(-1)
        return values

    def check_target_boundaries(self, expected_value, boundaries=("left", "right")):
        for boundary in boundaries:
            coordinates = ff.readmesh(
                "child_case", boundary=boundary, precision=17, verbose=False
            )
            points = np.column_stack(coordinates)
            for time in (1, 2, 3):
                with self.subTest(boundary=boundary, time=time):
                    actual = self.read_values(time, boundary)
                    # OpenFOAM may write a uniform value instead of an explicit list.
                    if actual.size == 1:
                        actual = np.full(len(points), actual[0])
                    expected = expected_value(points, time)
                    np.testing.assert_allclose(actual, expected, atol=1e-12, rtol=0)

    def test_default_boundaries(self):
        """Check that not specifying references all boundaries in accessor"""
        self.check_target_boundaries(
            source_value,
            boundaries=("left", "right", "bottom", "top", "front", "back"),
        )
        # Boundary defaults must not enable volume transfer.
        for time in (1, 2, 3):
            np.testing.assert_array_equal(self.read_values(time), -99.0)

    def test_empty_boundaries(self):
        """Check boundary field remains unchanged"""
        for boundary in (None, "left", "right", "bottom", "top", "front", "back"):
            for time in (1, 2, 3):
                with self.subTest(boundary=boundary, time=time):
                    values = self.read_values(time, boundary)
                    np.testing.assert_array_equal(values, -99.0)
