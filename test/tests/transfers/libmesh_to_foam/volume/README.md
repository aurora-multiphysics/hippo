# Volume transfers

`main.i` uses a 3 x 4 x 3 generated libMesh volume mesh with FIRST LAGRANGE source
values and a 4 x 3 x 2 OpenFOAM destination. Shape evaluation reproduces the
spatially linear, time-dependent analytical function at cell and face centres.

Serial volume-only runs explicitly clear the boundary list and verify unchanged
boundaries. The MPI 4-parent/2-child case transfers both internal values and all
physical boundaries. A nearest-node case uses the same nonmatching meshes with
unique nearest source nodes. Python checks values at times 1, 2, and 3.

The missing-internal-storage case targets `boundary_only` in the child and expects
a collective error, including on parent ranks that do not host the child.
