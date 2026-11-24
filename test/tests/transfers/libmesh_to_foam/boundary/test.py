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

    def test_shape(self):
        self.check_target_boundaries(source_value)

    def test_nearest_nodes(self):
        # Parent mirror: both x-normal patches of a 4 x 3 x 2 unit-cube mesh.
        x = (0.0, 1.0)
        y = np.linspace(0.0, 1.0, 4)
        z = np.linspace(0.0, 1.0, 3)
        samples = np.array(list(product(x, y, z)))

        def expected_value(points, time):
            # The child face centers have unique nearest parent nodes.
            distances = np.sum((points[:, None, :] - samples[None, :, :]) ** 2, axis=2)
            nearest = samples[np.argmin(distances, axis=1)]
            return source_value(nearest, time)

        self.check_target_boundaries(expected_value)

    def test_nearest_centroids(self):
        # The left and right faces are at 0 and 1
        x = (0.0, 1.0)
        # centroid will be in the middle of the element
        y = (np.arange(3) + 0.5) / 3  # (0.5 1.5 2.5)/3 unit length split in 3
        z = (np.arange(2) + 0.5) / 2  # (0.5 1.5)/2 unit length split in 2
        samples = np.array(list(product(x, y, z)))

        def expected_value(points, time):
            # No tie crosses the two-sample cutoff for this mesh pair. Averaging these
            # centroids differs from both nearest-one sampling and interpolation.
            distances = np.sum((points[:, None, :] - samples[None, :, :]) ** 2, axis=2)
            nearest_two = samples[np.argsort(distances, axis=1)[:, :2]]

            # evaluate at closest 2
            return np.mean(source_value(nearest_two, time), axis=1)

        self.check_target_boundaries(expected_value)
