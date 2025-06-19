import unittest
from pathlib import Path

from analytical import unsteady1d_temp
from read_hippo_data import read_moose_exodus_data, read_openfoam_data

import numpy as np

RUN_DIR = Path(__file__).parent
FOAM_CASE = RUN_DIR / "fluid-openfoam"
K_SOLID = 1
RHO_CP_SOLID = 1
K_FLUID = 4
RHO_CP_FLUID = 16
T_HOT = 2.
T_COLD = 1.
L = 1


class TestUnsteadyHeatConductionInInfiniteSystem(unittest.TestCase):

    def test_matches_analytic_solution_at_times(self):
        times = [0.0025, 0.005, 0.01]  # seconds
        for time in times:
            moose_coords, moose_temperature = read_moose_exodus_data(RUN_DIR / "run_out.e",
                                                                time,
                                                                "temp")
            foam_coords, foam_temperature = read_openfoam_data(FOAM_CASE,
                                                          time,
                                                          'T')
            x = np.concatenate([moose_coords['x'], foam_coords['x']])
            temp = np.concatenate([moose_temperature, foam_temperature])

            analytic_temp = unsteady1d_temp(
                x=x,
                time=time,
                temp_cold=T_COLD,
                temp_hot=T_HOT,
                k1=K_SOLID,
                k2=K_FLUID,
                rho_cp1=RHO_CP_SOLID,
                rho_cp2=RHO_CP_FLUID,
            )

            rmse = np.sqrt(np.sum(np.square(analytic_temp - temp)) / len(temp))
            self.assertLess(rmse, 5e-3, msg=f"for time = {time} s")
