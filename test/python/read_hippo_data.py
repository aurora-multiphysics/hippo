
from pathlib import Path
from typing import Literal
import numpy as np
import pyvista as pv

def read_moose_exodus_data(exo_file,
                        time,
                        variable,
                        block: int | str=0):

    reader: pv.ExodusIIReader = pv.get_reader(exo_file)
    reader.set_active_time_value(time)

    if block != 0 or block != 'Element blocks':
        reader.node_sets.enable_all()
        reader.side_sets.enable_all()
        reader.node_sets.enable_all_arrays()
        reader.side_sets.enable_all_arrays()

    data: pv.UnstructuredGrid = reader.read()[block].combine(True)

    if variable in data.point_data.keys():
        coords = data.points
        foam_variable = data.point_data[variable]
    elif variable in data.cell_data.keys():
        coords = data.cell_centers().points
        foam_variable = data.cell_data[variable]

    else:
        raise KeyError(f"Variable {variable} not found")

    foam_coords = dict(zip(('x', 'y', 'z'), coords.T))
    return foam_coords, foam_variable

def read_openfoam_data(case_dir: Path | str | bytes,
                       time: float,
                       variable: str,
                       block: str = 'internalMesh',
                       case_type: Literal['decomposed', 'reconstructed'] = 'reconstructed'
                       ) -> tuple[np.ndarray, np.ndarray]:

    file_name = Path(case_dir) / "case.foam"
    file_name.touch()

    reader: pv.POpenFOAMReader = pv.get_reader(file_name)
    reader.case_type = case_type
    reader.set_active_time_value(time)
    data: pv.UnstructuredGrid = reader.read()[block]

    coords = data.cell_centers().points
    foam_variable = data.cell_data[variable]
    foam_coords = dict(zip(('x', 'y', 'z'), coords.T))
    return foam_coords, foam_variable
