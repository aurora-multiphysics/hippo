# Boundary selection

Both transfer blocks target the same native field; only one is active per run.
`to_foam_default` omits `foam_boundaries`. Its serial and MPI runs widen source
support to all six physical patches and use FIRST LAGRANGE interpolation. Checks
cover all physical boundaries and unchanged internal values. Processor patches
must remain excluded because their interior points are outside the source surface.

The empty-selection run activates `to_foam` and explicitly clears its boundary
list. Boundary and internal values must remain at the initial value of -99.
