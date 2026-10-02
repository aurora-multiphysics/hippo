"""Verify direct initialization and subsequent fixed-value BC relaxation."""

import unittest

import fluidfoam as ff
import numpy as np
from read_hippo_data import get_foam_times


class TestFixedValueRelaxation(unittest.TestCase):
    def test_relaxation(self):
        case = "foam/"
        times = [t for t in get_foam_times(case, string=True) if float(t) > 0]
        self.assertEqual(len(times), 2, "Expected exactly two written timesteps")
        times.sort(key=float)
        np.testing.assert_allclose(
            [float(t) for t in times], [0.01, 0.02], rtol=1e-7, atol=1e-12
        )

        # Both inputs start at 8 and switch to 16 at the first timestep.
        expected = ((8, 8), (12, 14), (16, 16))
        for bc_type, boundaries in (
            ("FoamFixedValueBC", ("left", "right", "top")),
            ("FoamFixedValuePostprocessorBC", ("bottom", "front", "back")),
        ):
            for boundary, values in zip(boundaries, expected):
                for time, value in zip(times, values):
                    with self.subTest(bc_type=bc_type, boundary=boundary, time=time):
                        actual = np.asarray(
                            ff.readof.readscalar(case, time, "T", boundary=boundary)
                        )
                        self.assertGreater(actual.size, 0, "Empty boundary data")
                        np.testing.assert_allclose(
                            actual,
                            value,
                            rtol=1e-7,
                            atol=1e-12,
                            err_msg=f"{bc_type}: boundary={boundary}, time={time}",
                        )
