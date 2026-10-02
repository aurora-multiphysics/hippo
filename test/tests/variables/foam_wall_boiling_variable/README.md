# Boiling wall heat flux

This integration test derives from the OpenFOAM 14
`tutorials/multiphaseEuler/wallBoilingIATE` case. It runs the real
`multiphaseEuler` solver, including IATE, wall boiling, bulk phase change,
and the tutorial's turbulence and thermophysical transport models.
The required thermodynamic tables are copied into `foam/constant`, so the
fixture does not depend on an installed tutorial directory.

Both phases use `multiphaseExternalTemperature` on `wall`, specifying the
same total mixture heat flux of **73890 W/m²**. The two specifications do
not double the heat input. Hippo's `FoamWallHeatFlux` convention gives a
positive value for heating the fluid. Radiation and ambient heat exchange
are absent. Boiling is already included through effective conductivity;
adding `qEvaporative` or `qQuenching` again would double count it.

## Short startup

The original tutorial delays heating until 0.5 s. Here heat is applied
immediately, with boundary relaxation 1, on a 40 × 10 cell wedge mesh.
At the initial pressure of 2.62 MPa, the saturation table gives 359.981 K.
Liquid starts at 358.981 K, gas at 359.981 K, and the wall at 364.981 K.
The run takes ten fixed steps of 0.0001 s with ten PIMPLE outer correctors
and five energy correctors. The last two steps, 0.0009 and 0.001 s, are checked.
This is a short transient regression, not a converged physical validation
of the tutorial's boiling correlations or heat-flux partition.

## Checks

For both checked times, the test requires:

- All 40 wall faces and the complete eleven-time Exodus output sequence.
- Finite fluxes, positive face areas, and matching OpenFOAM/Exodus geometry.
- Each face within 5% of the signed imposed flux and the area average within 1%.
- Finite, nonnegative boiling diagnostics, with average evaporation above 1%
  and evaporation plus quenching above 10% of the imposed flux.

The boiling thresholds prevent a non-boiling run from passing and ensure
sensitivity to explicitly adding the boiling contribution again.
`foamPostProcess` exports `wallBoilingProperty(qEvaporative)` and
`wallBoilingProperty(qQuenching)` from the saved model state without
advancing the solver. This is necessary because Hippo's solve loop does
not execute the function objects in `system/functions`.

## Running

From the repository root, with Hippo built:

```bash
source external/openfoam/OpenFOAM-14/etc/bashrc
unset FOAM_SIGFPE FOAM_SETNAN
./run_tests -j4 --re foam_wall_boiling_variable
```

The flat test directory shares one `foam/` case. Prerequisites enforce
serial setup → run → verification → clean parallel setup → four-rank run
→ reconstruction → parallel verification. Reconstruction selects only
the two checked times. Each setup preserves checked-in `foam/0` and
removes generated OpenFOAM and Exodus results, preventing stale outputs
and serial/parallel races.

The initial local OpenMPI run completed all seven stages in approximately
10 seconds. The current formula matched the imposed flux to floating-point
precision. Validation also temporarily restored the old additive boiling
formula to check that the numerical assertions reject double counting.
