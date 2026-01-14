# Fixed-point iterations

The main premise of most tests is that they are set-up such that a run with multiple
fixed-points per time step should give the exact same result as a run with a single
fixed-point iteration. These cases should converge after two iterations as the result
from both iterations should be the same. As a result, this checks the ability of Hippo
to successfully restore the original data under increasingly complex conditions.
There are three categories
- Simple unit-like tests with bespoke solvers designed to test specfic functionality.
  The solvers can be found in `test/OpenFOAM/modules`
  - `function_test`: Variable `T` set to current time and its time derivative calculated.
    Also checks that the time derivative is calculated as 0 on the first time step
    (a bit silly but its what openfoam does).
  - `ode_euler` and `ode_crank_nicolson` computes an ODE T' = t with the result check
    against analytical solution and reference single fixed-point solution.
  - `laplacian_*`: checks computation of Poisson equation with time varying source.
- Tests that solve simple physical problems
  - `unsteady_*`: checks the 1D unsteady heat conduction problem in an `infinite domain'
    under different condtions such as using subcycling and variable time step duration.
  - `flow_over_heated_plate`: checks that full fluid problem is restored correctly with
    gravity and variable properties.
- Tests to check restepping and restarts
  - `restart_heated_plate`: checks that problems using fixed-point iteration can be
    restarted from the checkpoint file.
  - `restep_fixed_point`: checks that problem work correctly after the time step is
    cut back on failure to converge.
- Regression tests with Picard iteration which actually uses fixed-point to create
  tighter coupling.
  - `heated_plate_converge`: the `flow_over_heated_plate` test with Picard iteration
  - `unsteady_fixed_point_conerge`: the `unsteady_heat_conduction` test with
    Picard iteration.
