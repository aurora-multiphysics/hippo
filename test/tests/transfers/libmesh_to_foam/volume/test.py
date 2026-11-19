"""Check native transfer output against independently constructed source samples."""

from itertools import product
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

    def check_internal_values(self, expected_value):
        coordinates = ff.readmesh("child_case", precision=17, verbose=False)
        points = np.column_stack(coordinates)
        for time in (1, 2, 3):
            with self.subTest(time=time):
                actual = self.read_values(time)
                if actual.size == 1:
                    actual = np.full(len(points), actual[0])
                np.testing.assert_allclose(
                    actual, expected_value(points, time), atol=1e-12, rtol=0
                )

    def test_volume_shape(self):
        self.check_internal_values(source_value)

    def test_volume_and_boundaries(self):
        self.check_internal_values(source_value)
        self.check_target_boundaries(
            source_value, boundaries=("left", "right", "bottom", "top", "front", "back")
        )

    def test_volume_nearest_nodes(self):
        # The 3 x 4 x 3 source grid gives unique nearest nodes at 4 x 3 x 2 target cell centres.
        samples = np.array(
            list(
                product(
                    np.linspace(0.0, 1.0, 4),
                    np.linspace(0.0, 1.0, 5),
                    np.linspace(0.0, 1.0, 4),
                )
            )
        )

        def expected_value(points, time):
            distances = np.sum((points[:, None, :] - samples[None, :, :]) ** 2, axis=2)
            nearest = samples[np.argmin(distances, axis=1)]
            return source_value(nearest, time)

        self.check_internal_values(expected_value)
