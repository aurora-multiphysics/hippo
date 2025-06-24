""" # analytical.py

Module contains analytical solutions used in the test scripts
"""

import numpy as np
from scipy.special import erfc

def unsteady1d_temp(x: np.ndarray,
                    time: float,
                    temp_cold: float,
                    temp_hot: float,
                    k1: float,
                    k2: float,
                    rho_cp1: float,
                    rho_cp2: float) -> np.ndarray:
    """Returns the analytical solution to 1D unsteady heat conduction problem.

    Parameters
    ----------
    x : np.ndarray
        array of x coordinates to evaluate solution
    time : float
        time to evaluate solution
    temp_cold : float
        Cold temperature
    temp_hot : float
        Hot temperautre
    k1 : float
        thermal conductivity for x<0
    k2 : float
        thermal conductivity for x>0
    rho_cp1 : float
        \\rho c_p for  x<0
    rho_cp2 : float
        \\rho c_p for  x>0

    Returns
    -------
    np.ndarray
        Temperature for x at time
    """

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
