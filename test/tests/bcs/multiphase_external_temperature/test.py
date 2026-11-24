"""Compare coupled results with the stock reference, then verify spatial flux."""

import unittest

import numpy as np

from foam_reader import FoamReader


PHASES = ("liquid", "gas")
HEAT_FLUX = 73890


class TestMultiphaseExternalTemperature(unittest.TestCase):
    def test_coupled(self):
        foam = FoamReader("foam")
        reference = FoamReader("reference")
        output_times = [t for t in foam.get_times(string=True) if float(t) > 0]
        np.testing.assert_allclose([float(t) for t in output_times], [0.01, 0.02])

        for time in output_times:
            for phase in PHASES:
                field = f"T.{phase}"
                for entry in ("value", "refValue", "refGradient", "valueFraction"):
                    with self.subTest(time=time, phase=phase, entry=entry):
                        expected = reference.read_patch_entry(
                            time, field, "wall", entry
                        )
                        self.assertTrue(np.all(np.isfinite(expected)))

                        np.testing.assert_allclose(
                            foam.read_patch_entry(time, field, "wall", entry),
                            expected,
                            rtol=1e-7,
                            atol=1e-6,
                        )

                np.testing.assert_allclose(
                    foam.read_patch_entry(time, field, "wall", "coupledHeatFlux"),
                    HEAT_FLUX,
                )

            with self.subTest(time=time, field="evaporation"):
                expected = reference.read_patch_entry(
                    time, "wallBoiling:mDot", patch="wall"
                )
                self.assertTrue(np.all(np.isfinite(expected)))
                np.testing.assert_allclose(
                    foam.read_patch_entry(time, "wallBoiling:mDot", patch="wall"),
                    expected,
                    rtol=1e-7,
                    atol=1e-7,
                )
                if time == output_times[-1]:
                    self.assertGreater(
                        np.max(expected), 0, "Reference must reach active boiling"
                    )

    def test_spatial_relaxation(self):
        foam = FoamReader("foam")
        coords, _ = foam.read_field("0.0001", "T.liquid", block="boundary/wall")
        x = coords["x"]
        initial = HEAT_FLUX * (0.8 + 0.2 * x / 3.5)
        expected = initial.copy()

        output_times = [t for t in foam.get_times(string=True) if float(t) > 0]
        np.testing.assert_allclose([float(t) for t in output_times], [0.0001, 0.0002])

        # The first update must relax against the fully imposed INITIAL input.
        for time in output_times:
            expected = 0.5 * expected + 0.5 * initial * (1 + float(time))
            for phase in PHASES:
                np.testing.assert_allclose(
                    foam.read_patch_entry(
                        time, f"T.{phase}", "wall", "coupledHeatFlux"
                    ),
                    expected,
                    rtol=1e-8,
                    atol=1e-6,
                )

                values = foam.read_patch_entry(time, f"T.{phase}", "wall")
                self.assertTrue(np.all(np.isfinite(values)))
