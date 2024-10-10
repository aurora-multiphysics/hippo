import unittest
from pathlib import Path

import fluidfoam as ff
import numpy as np


class TestFlowOverHeatedPipe(unittest.TestCase):
    T_INF = 300  # K
    T_HOT = 310  # K

    def test_dimensionless_temperature_matches_precice_example(self):
        """
        Verify temperatures along the interface match reference data.

        The PreCICE example this test is based off uses a quantity θ,
        where θ = (T - T_inf)/(T_hot - T_inf) for its validation. It
        tracks the variation of θ along the fluid-solid interface for
        increasing x. I have exported these values and this test takes
        the RMSE between those reference values and the equivalent
        values across the OpenFOAM boundary.
        """
        foam_t = ff.readfield(
            "fluid-openfoam",
            time_name="1",
            name="T",
            boundary="interface",
            verbose=False,
        )
        theta = (foam_t - self.T_INF) / (self.T_HOT - self.T_INF)
        run_dir = Path.cwd()
        gold_dir = run_dir / "gold"
        ref_data = np.genfromtxt(gold_dir / "theta.csv", skip_header=1, delimiter=",")
        ref_theta = ref_data[:, 2][1:-1]

        self.assertEqual(theta.shape, ref_theta.shape)

        rms_error = np.sqrt(np.square(theta - ref_theta).sum() / len(theta))

        self.assertLess(rms_error, 5e-3)
