import unittest
from pathlib import Path

import fluidfoam as ff
import numpy as np

ROOT_DIR = Path(__file__).parent
BOTTOM_CASE = "fluid-bottom-openfoam"
TOP_CASE = "fluid-top-openfoam"
REF_TIME = "1"  # second


class TestSimplifiedHeatExchanger(unittest.TestCase):

    def test_bottom_interface_temperature_matches_reference(self):
        data = read_openfoam_interface_boundary_1(ROOT_DIR / BOTTOM_CASE, REF_TIME)
        gold = read_gold_csv(ROOT_DIR / "gold" / f"{BOTTOM_CASE}.csv")
        self.assertLess(rms_error(data[:, -1], gold[:, -1]), 5e-4)

    def test_top_interface_temperature_matches_reference(self):
        data = read_openfoam_interface_boundary_1(ROOT_DIR / TOP_CASE, REF_TIME)
        gold = read_gold_csv(ROOT_DIR / "gold" / f"{TOP_CASE}.csv")
        self.assertLess(rms_error(data[:, -1], gold[:, -1]), 5e-4)


def read_gold_csv(file: Path) -> np.ndarray:
    return np.genfromtxt(file, delimiter=",", skip_header=1)


def read_openfoam_interface_boundary_1(case_dir: Path, time_name: str) -> np.ndarray:
    coords = ff.readmesh(str(case_dir), boundary="interface", verbose=False)
    temp = ff.readfield(
        str(case_dir),
        time_name=time_name,
        name="T",
        boundary="interface",
        verbose=False,
    )
    return np.vstack([coords, temp]).T


def rms_error(a: np.ndarray, b: np.ndarray) -> float:
    if len(a) != len(b):
        raise ValueError(
            f"cannot calculate RMS error, len(a) '{len(a)}' != len(b) '{len(b)}'"
        )
    return np.sqrt(np.sum(np.square(a - b))) / len(a)


if __name__ == "__main__":
    import matplotlib.pyplot as plt

    gold_top = read_gold_csv(ROOT_DIR / "gold" / f"{TOP_CASE}.csv")
    foam_top = read_openfoam_interface_boundary_1(ROOT_DIR / TOP_CASE, REF_TIME)
    gold_bottom = read_gold_csv(ROOT_DIR / "gold" / f"{BOTTOM_CASE}.csv")
    foam_bottom = read_openfoam_interface_boundary_1(ROOT_DIR / BOTTOM_CASE, REF_TIME)

    print(f"Top RMSE:    {rms_error(gold_top[:, -1], foam_top[:, -1]):e}")
    print(f"Bottom RMSE: {rms_error(gold_bottom[:, -1], foam_bottom[:, -1]):e}")

    _, axs = plt.subplots(ncols=2)
    axs[0].plot(gold_top[:, 0], gold_top[:, -1], label="gold")
    axs[0].plot(foam_top[:, 0], foam_top[:, -1], label="OpenFOAM")
    axs[0].legend()
    axs[1].plot(gold_bottom[:, 0], gold_bottom[:, -1], label="gold")
    axs[1].plot(foam_bottom[:, 0], foam_bottom[:, -1], label="OpenFOAM")
    axs[1].legend()
    plt.show()
