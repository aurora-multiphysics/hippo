"""Exercise the OpenFOAM-backed readers using an asymmetric one-cell mesh."""

from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

import numpy as np

from foam_reader import FoamReader
from read_hippo_data import (
    get_foam_times,
    read_openfoam_data,
    read_openfoam_patch_entry,
)


def write_dictionary(path, body, kind="dictionary"):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        f"FoamFile {{ format ascii; class {kind}; object {path.name}; }}\n{body}\n"
    )


class TestOpenFoamReader(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.directory.cleanup)
        cls.case = Path(cls.directory.name)
        write_dictionary(
            cls.case / "system/controlDict",
            """
            application foamPostProcess;
            startFrom startTime; startTime 0; stopAt endTime; endTime 1;
            deltaT 1; writeControl timeStep; writeInterval 1;
            writeFormat ascii; writePrecision 17;
        """,
        )
        write_dictionary(cls.case / "system/fvSchemes", "")
        write_dictionary(cls.case / "system/fvSolution", "")
        write_dictionary(
            cls.case / "system/blockMeshDict",
            """
            convertToMeters 1;
            vertices ((0 0 0) (2 0 0) (1 1 0) (0 1 0)
                      (0 0 1) (2 0 1) (1 1 1) (0 1 1));
            blocks (hex (0 1 2 3 4 5 6 7) (1 1 1) simpleGrading (1 1 1));
            edges ();
            boundary (wall { type wall; faces
                ((0 4 7 3) (1 2 6 5) (0 1 5 4) (3 7 6 2) (0 3 2 1) (4 5 6 7)); });
            mergePatchPairs ();
        """,
        )
        (cls.case / "0").mkdir()
        subprocess.run(
            ["blockMesh", "-case", str(cls.case)], check=True, capture_output=True
        )
        control = cls.case / "system/controlDict"
        cls.original_control = control.read_bytes()
        cls.reader = FoamReader(cls.case)

    def test_constructor_preserves_control(self):
        self.assertEqual(
            (self.case / "system/controlDict").read_bytes(), self.original_control
        )
        self.assertFalse((self.case / "0/C").exists())

    def test_centres_generated_once(self):
        with patch("foam_reader.subprocess.run") as run:
            FoamReader(self.case)
            run.assert_not_called()

    def test_failed_generation_restores_control(self):
        with tempfile.TemporaryDirectory() as directory:
            case = Path(directory)
            (case / "constant/polyMesh").mkdir(parents=True)
            write_dictionary(case / "system/controlDict", "writePrecision 6;")
            original = (case / "system/controlDict").read_bytes()
            error = subprocess.CalledProcessError(
                1, "foamPostProcess", stderr="failure"
            )
            with patch(
                "foam_reader.subprocess.run", side_effect=[None, None, None, error]
            ):
                with self.assertRaisesRegex(RuntimeError, "Cannot generate centres"):
                    FoamReader(case)
            self.assertEqual((case / "system/controlDict").read_bytes(), original)

    def field(
        self,
        name="T",
        internal="uniform 1.0000000000000011",
        entries=None,
        kind="volScalarField",
    ):
        if entries is None:
            entries = "value uniform 1.0000000000000011; valueFraction uniform 0.25;"
        path = self.case / "0" / name
        write_dictionary(
            path,
            f"dimensions [0 0 0 0 0 0 0]; internalField {internal}; "
            f"boundaryField {{ wall {{ type calculated; {entries} }} }}",
            kind,
        )
        return path

    def test_double_precision_and_centres(self):
        self.field()
        coords, values = read_openfoam_data(self.case, 0, "T")
        expected = np.float64("1.0000000000000011")
        self.assertEqual(values.dtype, np.float64)
        self.assertEqual(values[0], expected)
        self.assertNotEqual(values[0], np.float64(np.float32(expected)))
        # Volume centroid differs from the mean of this trapezoid's vertices.
        np.testing.assert_allclose(
            [coords[a][0] for a in "xyz"], [7 / 9, 4 / 9, 0.5], rtol=0, atol=1e-15
        )
        for axis in coords.values():
            self.assertEqual(axis.dtype, np.float64)
        coords, values = read_openfoam_data(self.case, "0", "T", block="boundary/wall")
        self.assertEqual(values.shape, (6,))
        np.testing.assert_array_equal(values, np.full(6, expected))
        np.testing.assert_allclose(
            [coords[a][4] for a in "xyz"], [7 / 9, 4 / 9, 0], rtol=0, atol=1e-15
        )

    def test_patch_entries(self):
        self.field(
            entries="valueFraction uniform 0.25; value uniform 2; "
            "coupledHeatFlux nonuniform List<scalar> 3(1.0000000000000011 2 3);"
        )
        np.testing.assert_array_equal(
            read_openfoam_patch_entry(self.case, 0, "T", "wall"), [2]
        )
        np.testing.assert_array_equal(
            read_openfoam_patch_entry(self.case, 0, "T", "wall", "valueFraction"),
            [0.25],
        )
        np.testing.assert_array_equal(
            read_openfoam_patch_entry(self.case, 0, "T", "wall", "coupledHeatFlux"),
            [1.0000000000000011, 2, 3],
        )
        with self.assertRaisesRegex(ValueError, "Cannot read.*missing"):
            read_openfoam_patch_entry(self.case, 0, "T", "wall", "missing")
        self.field(entries="valueFraction uniform 0.25;")
        with self.assertRaises(ValueError):
            read_openfoam_patch_entry(self.case, 0, "T", "wall")

    def test_repeated_lists(self):
        self.field(entries="value nonuniform List<scalar> 6(2 2 2 2 2 2);")
        np.testing.assert_array_equal(
            read_openfoam_patch_entry(self.case, 0, "T", "wall"), np.full(6, 2)
        )
        self.field(
            "U",
            "uniform (1 2 3)",
            "value nonuniform List<vector> 2{(1 2 3)};",
            "volVectorField",
        )
        np.testing.assert_array_equal(
            read_openfoam_patch_entry(self.case, 0, "U", "wall"), [[1, 2, 3], [1, 2, 3]]
        )

    def test_case_bound_reader(self):
        self.field()
        reader = FoamReader(self.case)
        self.assertEqual(reader.get_times(string=True), ["0"])
        _, values = reader.read_field(0, "T")
        _, expected = read_openfoam_data(self.case, 0, "T")
        np.testing.assert_array_equal(values, expected)
        with tempfile.TemporaryDirectory() as directory:
            other = FoamReader(directory)
            self.assertEqual(other.get_times(), [])
            write_dictionary(
                Path(directory) / "1e-3/T",
                "dimensions [0 0 0 0 0 0 0]; internalField uniform 9; "
                "boundaryField { wall { type fixedValue; value uniform 9; } }",
                "volScalarField",
            )
            self.assertEqual(other.get_times(), [0.001])
            np.testing.assert_array_equal(
                other.read_patch_entry(0.001, "T", "wall"), [9]
            )
            self.assertEqual(reader.get_times(), [0])
        # Reusing a case directory must not return cached values from an earlier run.
        self.field(entries="value uniform 7;")
        np.testing.assert_array_equal(reader.read_patch_entry(0, "T", "wall"), [7])

    def test_vectors(self):
        self.field(
            "U",
            "nonuniform List<vector> 1((1.0000000000000011 2 3))",
            "value uniform (1 2 3);",
            "volVectorField",
        )
        _, values = read_openfoam_data(self.case, 0, "U")
        self.assertEqual(values.shape, (1, 3))
        np.testing.assert_array_equal(values, [[1.0000000000000011, 2, 3]])
        _, values = read_openfoam_data(self.case, 0, "U", block="boundary/wall")
        np.testing.assert_array_equal(values, np.tile([1, 2, 3], (6, 1)))
        self.field(
            "U",
            "uniform (1 2 3)",
            "value nonuniform List<vector> 2((1 2 3)(4 5 6));",
            "volVectorField",
        )
        np.testing.assert_array_equal(
            read_openfoam_patch_entry(self.case, 0, "U", "wall"), [[1, 2, 3], [4, 5, 6]]
        )

    def test_invalid_data(self):
        self.field(internal="nonuniform List<scalar> 2(1)")
        with self.assertRaisesRegex(ValueError, "T:internalField"):
            read_openfoam_data(self.case, 0, "T")
        self.field(internal="nonuniform List<scalar> 2(1 2)")
        with self.assertRaisesRegex(ValueError, "count mismatch"):
            read_openfoam_data(self.case, 0, "T")
        path = self.field()
        path.write_text(path.read_text().replace("format ascii", "format binary"))
        with self.assertRaisesRegex(ValueError, "ASCII"):
            read_openfoam_data(self.case, 0, "T")
        self.field(kind="volTensorField")
        with self.assertRaisesRegex(ValueError, "scalar/vector"):
            read_openfoam_data(self.case, 0, "T")
        with self.assertRaisesRegex(ValueError, "reconstructed"):
            read_openfoam_data(self.case, 0, "T", case_type="decomposed")
        with self.assertRaisesRegex(ValueError, "block"):
            read_openfoam_data(self.case, 0, "T", block="wall")
        with self.assertRaisesRegex(FileNotFoundError, "static mesh"):
            with tempfile.TemporaryDirectory() as directory:
                (Path(directory) / "0").mkdir()
                read_openfoam_data(directory, 0, "T")

    def test_times(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in ("0", "10", "2", "1e-3", "-1", "constant", "2backup"):
                (root / name).mkdir()
            (root / "3").touch()
            self.assertEqual(
                get_foam_times(root, string=True), ["-1", "0", "1e-3", "2", "10"]
            )
            self.assertEqual(get_foam_times(root), [-1, 0, 0.001, 2, 10])
            with self.assertRaisesRegex(ValueError, "Missing or ambiguous"):
                read_openfoam_patch_entry(root, 8, "T", "wall")
            (root / "0.001").mkdir()
            with self.assertRaisesRegex(ValueError, "Missing or ambiguous"):
                read_openfoam_patch_entry(root, 0.001, "T", "wall")
