"""Test module for FoamTimeStepper where OpenFOAM uses CFL adaptive time stepping"""

import os
import re
import subprocess

from unittest import TestCase


class TestFoamTimeStepper(TestCase):
    """Test class for checking correct times are run"""

    def test_synchronisation_and_cutback(self):
        """Checks synchronisation with parent app and ensure timestep recovery after cutback"""
        dirs = [dir for dir in os.listdir("fluid-openfoam") if re.search("0.*", dir)]
        for dir in [0.1, 0.2, 0.3, 0.4, 0.5]:
            assert str(dir) in dirs, f"{dir} resutlts folder not found"

        cutbacks = 0
        dirs = sorted(float(dir) for dir in dirs)
        for dir in [0.1, 0.2, 0.3, 0.4]:
            idx = dirs.index(dir)
            dt0 = dirs[idx - 1] - dirs[idx - 2]
            dt1 = dirs[idx] - dirs[idx - 1]
            dt2 = dirs[idx + 1] - dirs[idx]

            # only run the test for dir if the initial cutback is large enough
            if dt1 > 0.8 * dt0:
                continue

            cutbacks += 1
            assert dt2 > 1.25 * dt1, "Check recovery from cutback works properly"

        assert cutbacks > 0, (
            "Test has not worked properly there should be at least 1 cutback"
        )

    def test_force_no_cfl(self):
        """Checks that CFL is not used if dt is overriden"""
        dirs = os.listdir("fluid-openfoam")
        for t in [0.0, 0.1, 0.2, 0.3, 0.4]:
            for t1 in [0.03, 0.06, 0.09]:
                folder = f"{(t + t1):.2f}"
                assert folder in dirs, f"{folder} results folder not found"

    def test_foam_only(self):
        """Compare output times to foamRun, they should be the same."""
        dirs = [dir for dir in os.listdir("fluid-openfoam") if re.search("0.*", dir)]

        subprocess.run(
            ["foamCleanCase", "-case", "fluid-openfoam"],
            stdout=subprocess.DEVNULL,
            check=True,
        )
        subprocess.run(
            ["blockMesh", "-case", "fluid-openfoam"],
            stdout=subprocess.DEVNULL,
            check=True,
        )
        subprocess.run(
            ["foamRun", "-case", "fluid-openfoam"],
            stdout=subprocess.DEVNULL,
            check=True,
        )

        required = [
            dir for dir in os.listdir("fluid-openfoam") if re.search("0.*", dir)
        ]

        for dir in required:
            assert dir in dirs, f"Folder {dir} not found. dirs: {dirs}"
