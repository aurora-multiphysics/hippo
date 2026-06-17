"""Test module for FoamTimeStepper where OpenFOAM uses CFL adaptive time stepping"""

import os
import re

from unittest import TestCase


class TestFoamTimeStepper(TestCase):
    """Test class for checking correct times are run"""

    def test_adjustable_run_time(self):
        """Checks synchronisation with parent app and ensure timestep recovery after cutback"""
        dirs = [
            float(dir) for dir in os.listdir("fluid-openfoam") if re.search("0.*", dir)
        ]
        for dir in dirs:
            assert (dir % 0.12) < 1e-8, "Check adjustable runtime works"
