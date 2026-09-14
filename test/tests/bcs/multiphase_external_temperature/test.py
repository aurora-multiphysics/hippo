"""Compare coupled results with the stock reference, then verify spatial flux."""

import unittest

import fluidfoam as ff
import numpy as np

from patch_data import patch_entry, times


PHASES = ("liquid", "gas")
HEAT_FLUX = 73890


class TestMultiphaseExternalTemperature(unittest.TestCase):
    def test_coupled(self):
        output_times = times("foam")
        np.testing.assert_allclose([float(t) for t in output_times], [0.01, 0.02])

        for time in output_times:
            for phase in PHASES:
                field = f"T.{phase}"
                for entry in ("value", "refValue", "refGradient", "valueFraction"):
                    with self.subTest(time=time, phase=phase, entry=entry):
                        expected = patch_entry("reference", time, field, entry)
                        self.assertTrue(np.all(np.isfinite(expected)))
                        np.testing.assert_allclose(
                            patch_entry("foam", time, field, entry),
                            expected,
                            rtol=1e-7,
                            atol=1e-6,
                        )
                np.testing.assert_allclose(
                    patch_entry("foam", time, field, "coupledHeatFlux"), HEAT_FLUX
                )

            with self.subTest(time=time, field="evaporation"):
                expected = ff.readscalar(
                    "reference", time, "wallBoiling:mDot", boundary="wall"
                )
                self.assertTrue(np.all(np.isfinite(expected)))
                np.testing.assert_allclose(
                    ff.readscalar("foam", time, "wallBoiling:mDot", boundary="wall"),
                    expected,
                    rtol=1e-7,
                    atol=1e-7,
                )
                if time == output_times[-1]:
                    self.assertGreater(
                        np.max(expected), 0, "Reference must reach active boiling"
                    )

    def test_spatial_relaxation(self):
        x, _, _ = ff.readmesh("foam", boundary="wall")
        initial = HEAT_FLUX * (0.8 + 0.2 * x / 3.5)
        expected = initial.copy()
        self.assertGreater(np.ptp(initial), 0)
        output_times = times("foam")
        self.assertEqual(len(output_times), 2)
        np.testing.assert_allclose([float(t) for t in output_times], [0.0001, 0.0002])
        # The first update must relax against the fully imposed INITIAL input.
        for time in output_times:
            expected = 0.5 * expected + 0.5 * initial * (1 + float(time))
            for phase in PHASES:
                np.testing.assert_allclose(
                    patch_entry("foam", time, f"T.{phase}", "coupledHeatFlux"),
                    expected,
                    rtol=1e-8,
                    atol=1e-6,
                )
                values = patch_entry("foam", time, f"T.{phase}", "value")
                self.assertTrue(np.all(np.isfinite(values)))
