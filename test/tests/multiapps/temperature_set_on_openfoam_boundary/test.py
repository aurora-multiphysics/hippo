import unittest

import fluidfoam as ff
import numpy as np


class TestSimpleMultiApp(unittest.TestCase):
    LEFT_PATCH = "patch2"
    TOP_PATCH = "patch4"

    def test_openfoam_temperatures_match_reference(self):
        for patch in [self.LEFT_PATCH, self.TOP_PATCH]:
            temperature = ff.readfield(
                "buoyantCavity",
                time_name="0.1",
                name="T",
                boundary=patch,
                verbose=False,
            )
            coords = np.array(
                ff.readmesh(
                    "buoyantCavity",
                    boundary=patch,
                    verbose=False,
                )
            )
            expected_temp = self.calculate_expected_temperatures(coords.T)

            np.testing.assert_allclose(
                temperature, expected_temp, err_msg=f"mismatch in patch '{patch}'"
            )

    @staticmethod
    def calculate_expected_temperatures(coords: np.ndarray) -> np.ndarray:
        """
        Calculate the expected temperature at each of the given coords.

        Assumes that each row of the array is an [x, y, z] coordinate.

        Note that this function must do the same thing as in the
        '[Functions][sum_coords]' ParsedFunction block in 'run.i'.
        """
        return 300 + np.sum(coords, axis=1)
