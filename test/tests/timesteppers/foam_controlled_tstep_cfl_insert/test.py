"""Test module for FoamTimeStepper where OpenFOAM uses CFL adaptive time stepping"""

import os
from unittest import TestCase


class TestFoamTimeStepper(TestCase):
    """Test class for checking correct times are run"""

    def test_with_cfl(self):
        """Checks synchronisation with parent app"""
        dirs = os.listdir("fluid-openfoam")
        for dir in [0.1, 0.2, 0.3, 0.4, 0.5]:
            assert str(dir) in dirs, f"{dir} resutls folder not found"

    def test_force_no_cfl(self):
        """Checks that CFL is not used if dt is overriden"""
        dirs = os.listdir("fluid-openfoam")
        for t in [0.0, 0.1, 0.2, 0.3, 0.4]:
            for t1 in [0.03, 0.06, 0.09]:
                folder = f"{(t + t1):.2f}"
                assert folder in dirs, f"{folder} results folder not found"
