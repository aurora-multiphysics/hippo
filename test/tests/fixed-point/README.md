# Fixed-point iterations

The main premise of most tests is that they are set-up such that a run with multiple fixed-points per time step should give the exact same result as a run with a single fixed-point iteration. These cases should also converge after two iterations as the result from both iterations should be the same. As a result, this checks the ability of Hippo to successfully restore the original data under increasingly complex conditions. There are three categories
- Simple unit-like tests with bespoke solvers designed to test specfic functionality. The solvers can be found in `test/OpenFOAM/modules`
  - `function_test`: Variable `T` set to current time and its time derivative calculated. Also checks that the time derivative is calculated as 0 on the first time step (a bit silly but its what openfoam does).
  - `ode_euler` and `ode_crank_nicolson` computes an ODE T' = t with the result check against analytical solution and reference single fixed-point solution.
  - `laplacian_*`: checks computation of Poisson equation with time varying source.
- Tests that solve simple physical problems
  - `unsteady_*`: checks the 1D unsteady heat conduction problem in an `infinite domain' under different condtions such as using subcycling, variable time step duration.
  - `flow_over_flat_plate`: a parallel run on a full fluid problem (not currently working).
  - `flow_over_flat_plate_fftb`: a parallel run on a full fluid problem with FFTB coupling strategy (not currently working).
  - `shell_tube`: a parallel run on complex 3D problem.
- Regression test with Picard iteration which actually uses fixed-point to create tighter coupling.
  - `flow_over_flat_plate_picard`: a parallel run on a full fluid problem (not currently working).
