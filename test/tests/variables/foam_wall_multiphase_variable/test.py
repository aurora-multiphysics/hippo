"""Analytical phase-weighted quantities on all six cube boundaries."""

import unittest

import numpy as np
import pyvista as pv


class TestMultiphaseWallVariables(unittest.TestCase):
    def test_quantities(self):
        exo_file: pv.ExodusIIReader = pv.ExodusIIReader("main_out.e")

        times = np.asarray(exo_file.time_values)
        offsets = {
            "left": (0.125, 0, 0),
            "right": (-0.125, 0, 0),
            "bottom": (0, 0.125, 0),
            "top": (0, -0.125, 0),
            "front": (0, 0, 0.125),
            "back": (0, 0, -0.125),
        }
        for time in times[1:]:
            exo_file.set_active_time_value(time)
            mesh: pv.MultiBlock[pv.UnstructuredGrid] = exo_file.read()["Element Blocks"]

            for bc, offset in offsets.items():
                offset = np.array(offset)
                xyz = mesh[bc].cell_centers().points
                x, y, z = xyz.T
                cx, cy, cz = (xyz + offset).T

                tw = 315 + 1.75 * (x * y + y * z + z * x) * time
                bulk = 315 + 1.75 * (cx * cy + cy * cz + cz * cx) * time
                gradients = np.column_stack((y + z, x + z, x + y)) * time
                normal = -np.sign(offset)
                flux = 4.75 * np.sum(gradients * normal, axis=1)

                expected = {
                    "temperature": tw,
                    "flux": flux,
                    "bulk": bulk,
                    "htc": flux / (tw - 290),
                }

                # Compare results against reference
                for name, reference in expected.items():
                    np.testing.assert_allclose(
                        mesh[bc].cell_data[name],
                        reference,
                        rtol=1e-7,
                        atol=1e-12,
                        err_msg=f"{name} at time {time}",
                    )
