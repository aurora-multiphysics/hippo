# Boiling wall heat flux

This integration test derives from the OpenFOAM 14
`tutorials/multiphaseEuler/wallBoilingIATE` case. It runs the `multiphaseEuler`
solver, including the interfacial area transport equation (IATE) with wall
boiling, bulk phase change, and the models for turbulence and thermophysical
transport from the tutorial.

The heat flux is imposed by using `multiphaseExternalTemperature` on `wall` for
both phases, a heat flux of 73890 W/m2 is specified for each, which OpenFOAM
then partitions between the two.

## Short startup

We apply heat immediately, as opposed to the tutorial which delays heating until 0.5s.
The heat flux takes some time to converge and then we check the final two
timesteps (0.0009s and 0.001s).

We check that:
- there was substantial boiling.
- the heat flux copied by hippo to the patch is the value we imposed.

## Running

From the repository root, with Hippo built:

```bash
source external/openfoam/OpenFOAM-14/etc/bashrc
unset FOAM_SIGFPE FOAM_SETNAN
./run_tests -C test/tests/variables/foam_wall_boiling_variable/
```
