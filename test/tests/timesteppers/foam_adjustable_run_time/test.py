"""Test module for FoamTimeStepper where adjustableRunTIme is set for writeControl"""

import os
import re

from unittest import TestCase


class TestFoamTimeStepperAdjustableRunTime(TestCase):
    """Test class for checking correct times are run"""

    def test_adjustable_run_time(self):
        """Checks output folders match those expected from the controlDict"""
        dirs = [
            float(dir) for dir in os.listdir("fluid-openfoam") if re.search("0.*", dir)
        ]
        for dir in dirs:
            assert (dir % 0.12) < 1e-8, "Check adjustable runtime works"
