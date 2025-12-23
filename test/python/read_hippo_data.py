"""Helper functions for reading OpenFOAM and MOOSE exodus data
"""

import os
import re

from pathlib import Path
from typing import Literal

import numpy as np
import pyvista as pv

def get_exodus_times(exo_file: Path | str | bytes) -> list[float]:
    """Get the times from the exodus file.

    Parameters
    ----------
    exo_file : Path | str | bytes
        Exodus II file name/path

    Returns
    -------
    list[float]
        List of times
    """
    reader: pv.ExodusIIReader = pv.get_reader(exo_file)
    return reader.time_values

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

def read_moose_exodus_data(exo_file: Path | str | bytes,
                        time: float,
                        variable: str,
                        block: int | str=0) -> tuple[dict[str, np.ndarray], np.ndarray]:
    """Read Exodus file and return coordinate and variable data

    Parameters
    ----------
    exo_file : Path | str | bytes
        Exodus II file name
    time : float
        Time value
    variable : str
        Name of variable
    block : int | str, optional
        Block of Exodus II file to read, by default 0 (Element Blocks)

    Returns
    -------
    tuple[dict[str, np.ndarray], np.ndarray]
        Dictionary of coordinates and variable array

    Raises
    ------
    KeyError
        If variable is not found in Exodus file
    """
    reader: pv.ExodusIIReader = pv.get_reader(exo_file)
    reader.set_active_time_value(time)

    if block != 0 or block != 'Element blocks':
        reader.node_sets.enable_all()
        reader.side_sets.enable_all()
        reader.node_sets.enable_all_arrays()
        reader.side_sets.enable_all_arrays()

    data: pv.UnstructuredGrid = reader.read()[block].combine(merge_points=True)

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
                       ) -> tuple[dict[str, np.ndarray], np.ndarray]:
    """Read OpenFOAM data file and return coordinate and variable data

    Parameters
    ----------
    case_dir : Path | str | bytes
        OpenFOAM results directory
    time : float
        time
    variable : str
        Name of variable
    block : str, optional
        Which part of the mesh to read, by default 'internalMesh'
    case_type : Literal[decomposed, reconstructed], optional
        Whether data has been reconstructed or not, by default 'reconstructed'

    Returns
    -------
    tuple[dict[str, np.ndarray], np.ndarray]
        Dictionary of coordinates and variable array
    """
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
