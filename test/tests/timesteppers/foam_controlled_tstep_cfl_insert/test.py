"""Test module for FoamTimeStepper where OpenFOAM uses CFL adaptive time stepping"""

import os
from unittest import TestCase


class TestFoamTimeStepper(TestCase):
    """Test class for checking correct times are run"""

    def test_synchronisation(self):
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

    def test_foam_only(self):
        required = [
            "0",
            "0.1690917508793",
            "0.356509290831281",
            "0.0248832",
            "0.207397060458416",
            "0.393087970854348",
            "0.05474304",
            "0.245209305564808",
            "0.429481403211603",
            "0.090574848",
            "0.282623577241572",
            "0.465724948638813",
            "0.130213735998884",
            "0.319703708185321",
            "0.501849350445028",
        ]
        dirs = os.listdir("fluid-openfoam")
        for dir in required:
            assert dir in dirs, f"Folder {dir} not found"
