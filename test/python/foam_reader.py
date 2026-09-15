"""Double-precision OpenFOAM case reader backed by OpenFOAM utilities."""

import os
import re
import subprocess
from pathlib import Path
from typing import Literal

import numpy as np


def get_foam_times(
    case_dir: Path | str | bytes, string=False
) -> list[str] | list[float]:
    """Return numeric time directories in numeric order, including zero."""
    pattern = r"[+-]?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?"
    names = sorted(
        (
            p.name
            for p in Path(os.fsdecode(case_dir)).iterdir()
            if p.is_dir() and re.fullmatch(pattern, p.name)
        ),
        key=float,
    )
    return names if string else [float(name) for name in names]


class FoamReader:
    """Read one static reconstructed OpenFOAM case as float64 arrays.

    The OpenFOAM environment must be configured. Construction generates constant/C
    once when a mesh is present and the centre field is missing. Reads are uncached so subsequent solver output
    and resets of the same case directory are visible to an existing reader.
    """

    def __init__(
        self,
        case_dir: Path | str | bytes,
        case_type: Literal["decomposed", "reconstructed"] = "reconstructed",
    ):
        if case_type != "reconstructed":
            raise ValueError("Only reconstructed OpenFOAM cases are supported")

        self.case_dir = Path(os.fsdecode(case_dir)).resolve()

        centres = self.case_dir / "constant/C"
        if not centres.exists() and (self.case_dir / "constant/polyMesh").is_dir():
            self._generate_centres()

    def _generate_centres(self) -> None:
        """Write static centres at double precision and restore solver settings."""
        control = self.case_dir / "system/controlDict"
        original = control.read_bytes()
        command = []
        try:
            control_dict_updates = (
                ("writePrecision", "17"),
                ("writeFormat", "ascii"),
                ("writeCompression", "off"),
            )
            for entry, value in control_dict_updates:
                command = [
                    "foamDictionary",
                    str(control),
                    "-entry",
                    entry,
                    "-set",
                    value,
                ]
                subprocess.run(command, check=True, capture_output=True, text=True)

            # Creates cell centres in the constant/C directory
            command = [
                "foamPostProcess",
                "-case",
                str(self.case_dir),
                "-constant",
                "-noZero",
                "-func",
                "writeCellCentres",
            ]
            subprocess.run(command, check=True, capture_output=True, text=True)

        except subprocess.CalledProcessError as error:
            (self.case_dir / "constant/C").unlink(missing_ok=True)
            raise RuntimeError(
                f"Cannot generate centres in {self.case_dir}: {error.stderr.strip()}"
            ) from error
        finally:
            control.write_bytes(original)

    def get_times(self, string=False) -> list[str] | list[float]:
        """Return numeric time directories in numeric order, including zero."""
        return get_foam_times(self.case_dir, string=string)

    def read_patch_entry(
        self,
        time: str | float,
        variable: str,
        patch: str,
        entry: str = "value",
    ) -> np.ndarray:
        """Read an exact ASCII patch entry as float64; uniform entries have length one."""
        path = self.case_dir / self._resolve_time(time) / variable
        return self._values(path, f"boundaryField/{patch}/{entry}")[0]

    def read_field(
        self,
        time: str | float,
        variable: str,
        block: str = "internalMesh",
    ) -> tuple[dict[str, np.ndarray], np.ndarray]:
        """Read static reconstructed ASCII fields and centres without precision loss.

        Construction creates constant/C using OpenFOAM's writeCellCentres function
        with writePrecision 17 when needed. Blocks are internalMesh or boundary/<patch>. Scalars have
        shape (N,) and vectors (N, 3); uniform values are expanded to the centre count.
        Field reads do not modify the case.
        """
        case = self.case_dir
        time_name = self._resolve_time(time)
        if any(
            (case / str(t) / "polyMesh").exists() for t in self.get_times(string=True)
        ):
            raise ValueError(f"Time-dependent meshes are not supported: {case}")

        if block == "internalMesh":
            entry = "internalField"

        elif (
            block.startswith("boundary/")
            and block.count("/") == 1
            and block.split("/")[1]
        ):
            entry = f"boundaryField/{block.split('/')[1]}/value"

        else:
            raise ValueError(f"Unsupported OpenFOAM block: {block}")

        centres = case / "constant/C"
        if not centres.is_file():
            raise FileNotFoundError(
                f"Missing {centres}; construct FoamReader after generating the static mesh"
            )

        coords, _ = self._values(centres, entry)
        if coords.ndim != 2:
            raise ValueError(f"Expected vector centres in {centres}:{entry}")

        values, uniform = self._values(case / time_name / variable, entry)
        if uniform:
            values = np.repeat(values, len(coords), axis=0)

        if len(values) != len(coords):
            raise ValueError(
                f"Coordinate/value count mismatch for {variable} in {case}:{entry}"
            )

        return dict(zip(("x", "y", "z"), coords.T)), values

    def _resolve_time(self, time: str | float) -> str:
        names = self.get_times(string=True)
        if isinstance(time, str) and time in names:
            return time

        matches = [name for name in names if float(name) == float(time)]
        if len(matches) != 1:
            raise ValueError(
                f"Missing or ambiguous time {time!r} in {self.case_dir}: {matches}"
            )

        return str(matches[0])

    @staticmethod
    def _entry(path: Path, entry: str) -> str:
        """Select an exact dictionary entry without modifying the case."""
        try:
            result = subprocess.run(
                [
                    "foamDictionary",
                    str(path),
                    "-entry",
                    entry,
                    "-value",
                    "-writePrecision",
                    "17",
                ],
                check=True,
                capture_output=True,
                text=True,
            )
        except FileNotFoundError as error:
            raise RuntimeError(
                "foamDictionary is unavailable; source the OpenFOAM environment"
            ) from error
        except subprocess.CalledProcessError as error:
            raise ValueError(
                f"Cannot read {path}:{entry}: {error.stderr.strip()}"
            ) from error
        return result.stdout.strip().rstrip(";").strip()

    @staticmethod
    def _check_field(path: Path) -> None:
        # Inspect the header before asking a dictionary reader to process binary data.
        with path.open("rb") as stream:
            header = stream.read(8192).split(b"}", 1)[0]

        if not re.search(rb"\bformat\s+ascii\s*;", header):
            raise ValueError(f"Only ASCII OpenFOAM fields are supported: {path}")
        if not re.search(rb"\bclass\s+vol(?:Scalar|Vector)Field\s*;", header):
            raise ValueError(f"Only scalar/vector volume fields are supported: {path}")

    def _values(self, path: Path, entry: str) -> tuple[np.ndarray, bool]:
        """Parse OpenFOAM's uniform/nonuniform scalar or vector serialization."""
        self._check_field(path)
        text = self._entry(path, entry)
        uniform = re.fullmatch(r"uniform\s+(.+)", text, re.S)
        nonuniform = re.fullmatch(
            r"nonuniform\s+List<(scalar|vector)>\s+(\d+)\s*\((.*)\)", text, re.S
        )

        compact = re.fullmatch(
            r"nonuniform\s+List<(scalar|vector)>\s+(\d+)\s*\{(.*)\}", text, re.S
        )

        try:
            if compact:
                vector = compact[1] == "vector"
                count = 1
                body = compact[3].strip()
            elif uniform:
                body = uniform[1].strip()
                vector = body.startswith("(")
                count = 1
            elif nonuniform:
                vector = nonuniform[1] == "vector"
                count = int(nonuniform[2])
                body = nonuniform[3].strip()
            else:
                raise ValueError("expected uniform or nonuniform scalar/vector data")

            if vector:
                groups = re.findall(r"\(([^()]*)\)", body)
                if len(groups) != count or re.sub(r"\([^()]*\)", "", body).strip():
                    raise ValueError("invalid vector list")

                rows = [[float(value) for value in group.split()] for group in groups]
                if any(len(row) != 3 for row in rows):
                    raise ValueError("expected three vector components")

                values = np.asarray(rows, dtype=np.float64).reshape(count, 3)
            else:
                values = np.asarray(
                    [float(value) for value in body.split()], dtype=np.float64
                )
                if values.size != count:
                    raise ValueError(f"expected {count} values, got {values.size}")

        except ValueError as error:
            raise ValueError(
                f"Invalid field data in {path}:{entry}: {error}"
            ) from error

        if compact:
            values = np.repeat(values, int(compact[2]), axis=0)
        return values, uniform is not None
