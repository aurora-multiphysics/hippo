# Heat Conduction in an Infinite System

This simulation consists of a solid and a fluid domain.
The domains have a shared interface at x = 0.
The analytical solution for pure diffusion in this configuration
was derived by Yoshida et al [1].

## Setup

```txt
    -------------------+-------------------    ↑
        Solid          |    Fluid              h
    -------------------+-------------------    ↓
    ←------- L -------→ ←------- L -------→
                       |
                     x = 0

```

|                  | Solid | Fluid |
|------------------|-------|-------|
| k $[W/m/K]$      | 1     | 4     |
| ρ.Cp $[J/m^3/K]$ | 1     | 16    |
| L $[m]$          | 1     | 1     |
| h $[m]$          | 0.1   | 0.1   |
| T $[K]$          | 1     | 0     |

All boundaries (other than the interface) have
zero gradient boundary conditions.
The idea here is that `L` is large enough such that the system is
effectively infinite in the time frame of the simulation.

## Analytical Solution

The analytic solution to the problem is:

$$
T(x, t) =
    \begin{cases}
        T_h
        - \frac{(T_h - T_c) \sqrt{k_2(\rho C_p)_2}}
            {\sqrt{k_1(\rho C_p)_1} + \sqrt{k_2 (\rho C_p)_2}}
        \text{erfc}(\frac{-x}{2\sqrt{k_1 / (\rho C_p)_1 t}}),
        &x < 0 \text{ (Solid medium)}\\

        T_c
        + \frac{(T_h - T_c) \sqrt{k_1(\rho C_p)_1}}
            {\sqrt{k_1(\rho C_p)_1}
        + \sqrt{k_2 (\rho C_p)_2}}
        \text{erfc}(\frac{x}{2\sqrt{k_2 / (\rho C_p)_2 t}}),
        &x > 0 \text{ (Fluid medium)}
    \end{cases}
$$

## References

[1] Hiroaki Yoshida, Takayuki Kobayashi, Hidemitsu Hayashi, Tomoyuki Kinjo,
Hitoshi Washizu, and Kenji Fukuzawa. Boundary condition at a two-phase
interface in the lattice boltzmann method for the convection diffusion
equation.
Physical Review E, 90(1):013303, 2014. doi: https://doi.org/10.1103/PhysRevE.90.013303.
