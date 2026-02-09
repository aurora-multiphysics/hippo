"""Script to visualise the shell and tube heat exchanger example"""

from pathlib import Path

import numpy as np
import pyvista as pv


def main():
    outer_file = Path("fluid-outer-openfoam/outer.foam")
    inner_file = Path("fluid-inner-openfoam/inner.foam")

    # Create OpenFOAM visualisation files
    outer_file.touch()
    inner_file.touch()

    outer_reader = pv.get_reader(outer_file)
    inner_reader = pv.get_reader(inner_file)
    solid_reader = pv.get_reader("solid_out.e")

    outer_reader.case_type = "decomposed"
    inner_reader.case_type = "decomposed"

    # Set visualisation time to be the last available time
    # and make sure the other domains use the closest time
    outer_reader.set_active_time_point(outer_reader.number_time_points - 1)

    target_time = outer_reader.active_time_value
    inner_point = np.argmin(abs(target_time - np.array(inner_reader.time_values)))
    inner_reader.set_active_time_point(inner_point)

    solid_point = np.argmin(abs(target_time - np.array(solid_reader.time_values)))
    solid_reader.set_active_time_point(solid_point)

    # Read and extract the volume meshes of each
    outer = outer_reader.read()["internalMesh"]
    inner = inner_reader.read()["internalMesh"]
    solid = solid_reader.read()["Element Blocks"]

    # Create the streamlines
    # Note that the source is random so the streamlines will look different
    # each time
    streamlines = outer.streamlines(
        vectors="U",
        source_center=(0, -0.85, -1.6),
        source_radius=0.12,
        n_points=50,
        integration_direction="forward",
        max_steps=5000,
    )

    # Plot the results
    p = pv.Plotter()

    kwargs = {"clim": (280, 353), "cmap": "jet", "show_scalar_bar": False}
    p.add_mesh(solid, scalars="T", **kwargs)
    p.add_mesh(inner, scalars="T", **kwargs)
    p.add_mesh(
        streamlines, scalars="T", render_lines_as_tubes=True, line_width=7.0, **kwargs
    )

    p.view_xy()
    p.camera.up = (0, -1, 0)
    p.camera.azimuth = 30
    p.camera.elevation = 20
    p.camera.zoom(2.0)

    p.add_scalar_bar("Temperature", position_x=0.5, position_y=0.1, width=0.4)
    p.show()


if __name__ == "__main__":
    main()
