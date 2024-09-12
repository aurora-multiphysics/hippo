import unittest

import fluidfoam as ff
import numpy as np


class TestSimpleMultiApp(unittest.TestCase):
    def test_openfoam_temperatures_match_reference(self):
        foam_t = ff.readfield(
            "buoyantCavity",
            time_name="0.1",
            name="T",
            verbose=False,
        )
        ref_foam_t = ff.readfield(
            "gold/buoyantCavity",
            time_name="0.1",
            name="T",
            verbose=False,
        )

        np.testing.assert_allclose(foam_t, ref_foam_t)
