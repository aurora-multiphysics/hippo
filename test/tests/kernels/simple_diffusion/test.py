"""Test module for basic MOOSE functionality within Hippo"""

from pathlib import Path
import unittest

from read_hippo_data import read_moose_exodus_data

RUN_DIR = Path(__file__).parent


def u_analytic(x):
    return x


class TestDiffusionProblem(unittest.TestCase):
    """Test that simple kernels work within Hippo"""

    def test_analytical_solution(self):
        """Test diffusion kernel against analytical solution"""
        coords, u = read_moose_exodus_data(RUN_DIR / "simple_diffusion_out.e", 1.0, "u")

        assert all(abs(u_analytic(coords["x"]) - u) < 3e-8), (
            f"Max diff: {abs(coords['x'] - u).max()}"
        )
