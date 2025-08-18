
from unittest import TestCase
from pathlib import Path
import pyvista as pv
import numpy as np

class TestFunctionSolver(TestCase):
    """Test fixed-point iteration where the solver imposes the current time on the internal field.
    """
    def test_times(self):
        """
        Test that internal field equals the current time and that old field is the previous time.

        This check that the time is correctly set given that the times must be
        restored from stream
        """

        # read from openfoam file
        case_dir = 'foam/'
        file_name = Path(case_dir) / "case.foam"
        file_name.touch()

        reader: pv.POpenFOAMReader = pv.get_reader(file_name)
        times = reader.time_values
        for i in range(len(times)):
            # set time and read internal field
            reader.set_active_time_point(i)
            data: pv.UnstructuredGrid = reader.read()['internalMesh']
            # check T array equals the time
            assert np.allclose(data.cell_data['T'], reader.time_values[i], atol=0, rtol=1e-12)

            # check that next `old time' is equal to the present time
            if i < len(times) -1:
                reader.set_active_time_point(i+1)
                data1: pv.UnstructuredGrid = reader.read()['internalMesh']
                assert np.allclose(data.cell_data['T'], data1.cell_data['T2'], atol=0, rtol=1e-12)

        file_name.unlink()
