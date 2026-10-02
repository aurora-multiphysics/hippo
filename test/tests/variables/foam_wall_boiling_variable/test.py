"""Check imposed mixture heat flux while the real wall-boiling model is active."""

from pathlib import Path
import subprocess
import unittest

import numpy as np
import pyvista as pv

from read_hippo_data import get_foam_times, get_exodus_times


IMPOSED_FLUX = 73890.0  # Positive into the fluid in Hippo's wall-gradient convention.
FOAM_TIMES = (0.0009, 0.001)


def get_nearest_time(times: list[float], time: float) -> float:
    return times[np.abs(np.array(times) - time).argmin()]


class TestBoilingWallHeatFlux(unittest.TestCase):
    def test_heat_flux(self):
        root = Path(__file__).resolve().parent
        # Hippo does not execute controlDict function objects during its solve loop.
        # Read the saved model state without advancing it, using OpenFOAM's exporter.
        subprocess.run(
            [
                "foamPostProcess",
                "-case",
                str(root / "foam"),
                "-solver",
                "multiphaseEuler",
                "-time",
                ",".join(f"{time:g}" for time in FOAM_TIMES),
                "-funcs",
                "(wallBoilingProperty(qEvaporative) wallBoilingProperty(qQuenching))",
            ],
            check=True,
        )
        exodus = pv.ExodusIIReader("main_out.e")

        foam_path = Path("foam/case.foam")
        foam_path.touch()

        foam = pv.OpenFOAMReader(foam_path)

        foam.cell_to_point_creation = False
        foam.disable_all_patch_arrays()
        foam.enable_patch_array("patch/wall")
        foam.disable_all_cell_arrays()
        properties = ("wallBoiling:qEvaporative", "wallBoiling:qQuenching")
        for name in properties:
            foam.enable_cell_array(name)

        for time in FOAM_TIMES:
            exo_time = get_nearest_time(get_exodus_times("main_out.e"), time)
            self.assertTrue(
                np.isclose(time, exo_time, atol=1e-12, rtol=0),
                f"Exodus times are not close: {time} vs {exo_time}",
            )

            exodus.set_active_time_value(exo_time)
            wall = exodus.read()["Element Blocks"][0]

            flux = wall.cell_data["flux"]
            area = wall.compute_cell_sizes().cell_data["Area"]
            foam_time = get_nearest_time(get_foam_times("foam"), time)
            self.assertTrue(
                np.isclose(time, foam_time, atol=1e-12, rtol=0),
                f"Foam times are not close: {time} vs {foam_time}",
            )
            foam.set_active_time_value(foam_time)

            reference_wall = foam.read()["boundary"]["wall"]
            reference_area = reference_wall.compute_cell_sizes().cell_data["Area"]

            evaporation = np.average(
                reference_wall.cell_data[properties[0]], weights=reference_area
            )
            quenching = np.average(
                reference_wall.cell_data[properties[1]], weights=reference_area
            )

            mean_flux = np.average(flux, weights=area)
            worst_error = np.max(np.abs(flux / IMPOSED_FLUX - 1))
            message = (
                f"t={time:g}: mean flux={mean_flux:.8g} W/m^2; "
                f"maximum face error={worst_error:.3%}; "
                f"evaporation={evaporation:.8g}, quenching={quenching:.8g} W/m^2"
            )

            # check the test actually has substantial boiling
            self.assertTrue(np.isfinite(evaporation), "Evaporation is invalid")
            self.assertTrue(np.isfinite(quenching), "Quenching is invalid")

            self.assertGreater(evaporation, 0.01 * IMPOSED_FLUX, message)
            self.assertGreater(evaporation + quenching, 0.10 * IMPOSED_FLUX, message)

            # check fluxes element wise
            np.testing.assert_allclose(
                flux, IMPOSED_FLUX, rtol=0.05, atol=0, err_msg=message
            )
            # use tighter check for face average heating
            self.assertLessEqual(abs(mean_flux / IMPOSED_FLUX - 1), 0.01, message)


if __name__ == "__main__":
    unittest.main()
