#!/usr/bin/env python3
"""
A script to generate reference data from the 'heat-exchanger-simplified'
example in the preCICE tutorials [1]. The example must have been run
before this script will work.

Two CSV files will be generated that contain the temperatures across the
interfaces of the heat exchanger. The temperatures are taken from the
boundaries of the OpenFOAM meshes.

[1] https://precice.org/tutorials-heat-exchanger-simplified.html
"""
import argparse
from dataclasses import dataclass
from pathlib import Path

import fluidfoam as ff
import numpy as np

BOUNDARY_NAME = "interface"
BOTTOM_CASE = "fluid-bottom-openfoam"
TOP_CASE = "fluid-top-openfoam"
REFERENCE_TIME = 1  # second


@dataclass
class CliArgs:
    tutorial_dir: Path
    output_dir: Path


def main(argv: list[str]) -> int:
    args = parse_args(argv[1:])
    foam_case = TOP_CASE
    for foam_case in [BOTTOM_CASE, TOP_CASE]:
        temperatures = read_openfoam_interface_boundary(
            args.tutorial_dir / foam_case, str(REFERENCE_TIME)
        )
        np.savetxt(
            args.output_dir / f"{foam_case}.csv",
            temperatures,
            fmt="%.6f",
            delimiter=",",
            header="x,y,z,T",
            comments="",
        )
    return 0


def parse_args(argv: list[str]) -> CliArgs:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        "tutorial_dir",
        type=Path,
        help="the path to the directory containing the heat exchanger example.",
    )
    parser.add_argument(
        "--output_dir",
        "-o",
        type=Path,
        required=False,
        default=Path.cwd(),
        help="the path to output the reference data.",
    )
    return CliArgs(**vars(parser.parse_args(argv)))


def read_openfoam_interface_boundary(case_dir: Path, time_name: str) -> np.ndarray:
    coords = ff.readmesh(str(case_dir), boundary=BOUNDARY_NAME, verbose=False)
    temp = ff.readfield(
        str(case_dir),
        time_name=time_name,
        name="T",
        boundary=BOUNDARY_NAME,
        verbose=False,
    )
    return np.vstack([coords, temp]).T


if __name__ == "__main__":
    import sys

    sys.exit(main(sys.argv))
