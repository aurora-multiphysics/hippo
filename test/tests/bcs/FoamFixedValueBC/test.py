"""Tests for shadowing scalar variables and function objects using MOOSE
"""
import re
import os
import unittest
import fluidfoam as ff
import numpy as np

def get_foam_times(case_dir: str | bytes) -> list[str]:
    """Get the times from the foam case directory

    Parameters
    ----------
    case_dir : str | bytes
        Case directory

    Returns
    -------
    list[str]
        Sorted list of times
    """
    return sorted(folder
                  for folder in os.listdir(case_dir)
                  if re.match(r"[-+]?([0-9]*\.[0-9]+|[0-9]+)", folder))

class TestFoamBCFixedValue(unittest.TestCase):
    """Test class for imposing fixed value BCs in Hippo."""
    def test_fixed_value(self):
        """Test case for shadowing a volScalarField."""
        case_dir = 'foam/'
        boundaries = ['top', 'bottom', 'front', 'back', 'left', 'right']
        times = get_foam_times(case_dir)[1:]

        for time in times:
            for boundary in boundaries:
                coords = dict(zip(('x','y','z'),ff.readof.readmesh(case_dir, boundary=boundary)))

                temp = ff.readof.readscalar(case_dir, time, "T", boundary=boundary)

                temp_ref = 0.05 + (coords['x'] + coords['y'] + coords['z'])*np.float64(time)
                temp_diff_max = np.argmax(abs(temp-temp_ref))
                assert np.allclose(temp_ref, temp, rtol=1e-7, atol=1e-12),\
                            (f"Max diff {boundary} ({time}): {abs(temp-temp_ref).max()} "
                             f"{temp[temp_diff_max]} {temp_ref[temp_diff_max]}")
