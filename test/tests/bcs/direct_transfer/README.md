# Direct BC transfer lifecycle

The focused `fixed_value`, `laplace_fixed_gradient` and `laplace_flux_bc` suites
compare direct and AuxVariable inputs against their existing solution references.
This suite checks transfer execution and checkpoint recovery.

INITIAL-only, time-varying and MPI transfer runs use `solve = false` in the child.
These runs check that transfers complete without errors or deadlocks. The child
mirrors only the bottom patch, so transfers also target patches outside its
mirrored mesh. The MPI run uses four parent ranks and two child ranks, with
front/back patches empty on one child rank. Numerical correctness is covered by
the focused BC suites and the recovery solution check. Invalid `v` input is
checked during setup.

Recovery is the only scenario that enables the OpenFOAM solve. It uses
INITIAL-only inputs for `T = 100 + x + y + z`, with
`--test-checkpoint-half-transient` to stop at time two of four, then resumes with
`--recover direct_transfer_cp/LATEST`. Transfers remain INITIAL-only during recovery,
so they cannot hide missing checkpoint data by overwriting the restored BC inputs.
The Python checker runs after recovery and compares the time-two checkpoint output and
recovered outputs at times three and four with the analytical solution.

Setup copies the local `case_template/` before meshing and decomposition. The
`bcTest` solver uses constant conductivity two. Test prerequisites serialize runs
that share case directories.
