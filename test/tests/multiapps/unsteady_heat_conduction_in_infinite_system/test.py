import unittest
from pathlib import Path

import fluidfoam as ff
import numpy as np
from scipy.special import erfc

RUN_DIR = Path(__file__).parent
FOAM_CASE = RUN_DIR / "fluid-openfoam"
K_SOLID = 1
RHO_CP_SOLID = 1
K_FLUID = 4
RHO_CP_FLUID = 16
T_HOT = 1
T_COLD = 1e-10
L = 1


class TestUnsteadyHeatConductionInInfiniteSystem(unittest.TestCase):

    def test_matches_analytic_solution_at_times(self):
        times = [0.0025, 0.005, 0.01]  # seconds
        for time in times:
            moose_x, moose_temperature = read_moose_temperature(RUN_DIR, time)
            foam_x, foam_temperature = read_openfoam_temperature(FOAM_CASE, time)
            x = np.concatenate([moose_x, foam_x])
            temp = np.concatenate([moose_temperature, foam_temperature])

            analytic_temp = calc_analytic_temp(
                x=x,
                time=time,
                temp_cold=T_COLD,
                temp_hot=T_HOT,
                k1=K_SOLID,
                k2=K_FLUID,
                rho_cp1=RHO_CP_SOLID,
                rho_cp2=RHO_CP_FLUID,
            )

            rmse = np.sqrt(np.sum(np.square(analytic_temp - temp)) / len(temp))
            self.assertLess(rmse, 5e-3, msg=f"for time = {time} s")


def read_moose_temperature(run_dir: Path, time: float) -> tuple[np.ndarray, np.ndarray]:
    times_csv = run_dir / "run_temp_over_line_csv_temp_over_line_time.csv"
    times = np.genfromtxt(times_csv, delimiter=",", skip_header=1)
    time_step = int(times[np.isclose(times[:, 0], time), 1][0])

    temperature_csv = (
        RUN_DIR / f"run_temp_over_line_csv_temp_over_line_{time_step:04d}.csv"
    )
    data = np.genfromtxt(temperature_csv, delimiter=",", skip_header=1)
    moose_x = data[:, 2]
    moose_temperature = data[:, 1]
    return moose_x, moose_temperature


def read_openfoam_temperature(
    case_dir: Path, time: float
) -> tuple[np.ndarray, np.ndarray]:
    time_name = str(time)
    for time_dir in map(
        lambda p: p.name, filter(lambda p: p.is_dir(), case_dir.glob("*"))
    ):
        try:
            f = float(time_dir)
        except (TypeError, ValueError):
            continue
        if np.isclose(f, time):
            time_name = time_dir
            break

    case = str(case_dir)
    coords = np.array(ff.readmesh(case, verbose=False))
    all_foam_temp = ff.readfield(case, name="T", time_name=time_name, verbose=False)
    center_z_pos = 0.05
    mid_line_idx = np.where(np.isclose(coords[1], center_z_pos))
    foam_x = coords[0, mid_line_idx].T
    foam_x = np.reshape(coords[0, mid_line_idx].T, (len(foam_x),))
    foam_temperature = all_foam_temp[mid_line_idx]
    return foam_x, foam_temperature


def calc_analytic_temp(
    x: np.ndarray,
    time: float,
    temp_cold: float,
    temp_hot: float,
    k1: float,
    k2: float,
    rho_cp1: float,
    rho_cp2: float,
) -> np.ndarray:
    def temp_minus(
        x: np.ndarray,
        time: float,
        temp_cold: float,
        temp_hot: float,
        k1: float,
        k2: float,
        rho_cp1: float,
        rho_cp2: float,
    ) -> np.ndarray:
        numerator = (temp_hot - temp_cold) * np.sqrt(k2 * rho_cp2)
        denominator = np.sqrt(k1 * rho_cp1) + np.sqrt(k2 * rho_cp2)
        erfc_arg = -x / (2 * np.sqrt(k1 / rho_cp1 * time))
        return temp_hot - (numerator / denominator) * erfc(erfc_arg)

    def temp_plus(
        x: np.ndarray,
        time: float,
        temp_cold: float,
        temp_hot: float,
        k1: float,
        k2: float,
        rho_cp1: float,
        rho_cp2: float,
    ) -> np.ndarray:
        numerator = (temp_hot - temp_cold) * np.sqrt(k1 * rho_cp1)
        denominator = np.sqrt(k1 * rho_cp1) + np.sqrt(k2 * rho_cp2)
        erfc_arg = x / (2 * np.sqrt(k2 / rho_cp2 * time))
        return temp_cold + (numerator / denominator) * erfc(erfc_arg)

    positive_x_idx = x >= 0
    temp = np.empty_like(x)
    temp[positive_x_idx] = temp_plus(
        x=x[positive_x_idx],
        time=time,
        temp_cold=temp_cold,
        temp_hot=temp_hot,
        k1=k1,
        k2=k2,
        rho_cp1=rho_cp1,
        rho_cp2=rho_cp2,
    )
    negative_x_idx = ~positive_x_idx
    temp[negative_x_idx] = temp_minus(
        x=x[negative_x_idx],
        time=time,
        temp_cold=temp_cold,
        temp_hot=temp_hot,
        k1=k1,
        k2=k2,
        rho_cp1=rho_cp1,
        rho_cp2=rho_cp2,
    )
    return temp


if __name__ == "__main__":
    import matplotlib.pyplot as plt
    from scipy.optimize import curve_fit

    # Here we leave some helpful debugging code that generates figures
    # to check this by eye.
    # We also perform a fit over the measured temperatures to check we
    # get the expected thermal properties (conductivity, rho*Cp) back
    # out.

    def find_nearest(array, value):
        idx = np.searchsorted(array, value, side="left")
        if idx > 0 and (
            idx == len(array)
            or np.fabs(value - array[idx - 1]) < np.fabs(value - array[idx])
        ):
            return idx - 1
        else:
            return idx

    _, axs = plt.subplot_mosaic([["1", "2"], ["3", "3"]])
    for i, time in enumerate([0.0025, 0.005, 0.01]):
        moose_x, moose_temperature = read_moose_temperature(RUN_DIR, time)
        foam_x, foam_temperature = read_openfoam_temperature(FOAM_CASE, time)
        # Sample down the OpenFOAM values so the plot is less busy
        dx = foam_x[-1] - foam_x[-2]
        foam_x2 = [foam_x[0]]
        foam_temperature2 = [foam_temperature[0]]
        for fi in range(1, len(foam_x)):
            next_value = foam_x2[-1] + dx
            dx *= 1.1
            idx = find_nearest(foam_x, next_value)
            if np.isclose(foam_x[idx], foam_x2[-1]):
                continue
            foam_x2.append(foam_x[idx])
            foam_temperature2.append(foam_temperature[idx])

        x_space = np.linspace(-L, L, 201)
        analytic_temp = calc_analytic_temp(
            x_space,
            time,
            T_COLD,
            T_HOT,
            K_SOLID,
            K_FLUID,
            RHO_CP_SOLID,
            RHO_CP_FLUID,
        )

        # Best fit
        fit_x = np.concatenate([moose_x, foam_x])
        fit_y = np.concatenate([moose_temperature, foam_temperature])
        fit_kwargs = {"time": time, "temp_cold": T_COLD, "temp_hot": T_HOT}
        popt, pcov = curve_fit(
            lambda x, k1, k2, rho_cp1, rho_cp2: calc_analytic_temp(
                x, k1=k1, k2=k2, rho_cp1=rho_cp1, rho_cp2=rho_cp2, **fit_kwargs
            ),
            fit_x,
            fit_y,
            p0=[K_SOLID, K_FLUID, RHO_CP_SOLID, RHO_CP_FLUID],
            bounds=([0, 0, 0, 0], [np.inf, np.inf, np.inf, np.inf]),
        )
        print(f"{popt=}")

        # Calculate RMS
        x = np.concatenate([moose_x, foam_x])
        y = np.concatenate([moose_temperature, foam_temperature])
        expected_y = calc_analytic_temp(x, time, T_COLD, T_HOT, 1, 4, 1, 16)
        rmse = np.sqrt(np.sum(np.square(y - expected_y)) / len(y))
        print(f"{time}s RMS error: {rmse:e}")

        ax = axs[str(i + 1)]
        ax.scatter(moose_x, moose_temperature, label="MOOSE", marker="v", color="brown")
        ax.scatter(
            foam_x2,
            foam_temperature2,
            label="OpenFOAM",
            marker="o",
            s=11,
            edgecolors="blue",
            facecolors="none",
            linewidth=1,
        )
        ax.plot(x_space, analytic_temp, "--", label="Analytic")
        ax.plot(
            x_space,
            calc_analytic_temp(x_space, time, T_COLD, T_HOT, *popt),
            "r-.",
            label="fit",
            linewidth=0.5,
        )
        ax.set_title(f"Time = {time} s")
        ax.set_xlabel(r"$x$ $[m]$")
        ax.set_ylabel(r"Temperature $[K]$")
        ax.legend()
    plt.show()
