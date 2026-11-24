# Boundary sampling

`main.i` selects `to_foam`, transferring to left/right patches. The source is
restricted to those two mirrored surfaces. Shape cases use matching 4 x 3 x 2
meshes; nearest cases use a 4 x 4 x 3 child mesh. CLI overrides select FIRST
LAGRANGE nodes or CONSTANT MONOMIAL centroids with two nearest samples.

Python checks current values at times 1, 2, and 3 against analytical source
samples. The nearest configurations avoid ties at the selection cutoff. Serial
and MPI 4-parent/2-child cases exercise transfer wiring and source-value updates.
