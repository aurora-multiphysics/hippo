# libMesh-to-OpenFOAM transfer tests

The tests are grouped by behavior:

| Directory | Coverage |
|---|---|
| `boundary/` | Shape, nearest-node, and nearest-centroid boundary transfers; serial and MPI 4/2 |
| `boundary_selection/` | Omitted and explicitly empty boundary lists; serial and MPI default selection |
| `volume/` | Volume-only shape and nearest-node transfers, combined MPI transfers, and missing internal storage |

Each group has its own `main.i`, `child.i`, `test.py`, and `tests` specification.
Inputs and analytical output checks are local to the group. Evaluator algorithm
coverage remains in the unit tests.

All groups share `case_template/` and `setup.sh`. The harness runs
`bash ../setup.sh [serial|mpi_4_2] [shape|nearest]` from the group directory, creating
that group's own `parent_case/` and `child_case/`. Existing cases there are replaced
on each setup. Prerequisites sequence runs within a group; no prerequisite crosses
group boundaries, so the groups can run independently or concurrently.

The `fieldTransferTest` solver preserves transferred values and writes them at
times 1, 2, and 3. MPI output is reconstructed before Python verification. In MPI
4/2 cases, two parent ranks do not host the child but still participate in source
evaluation. Volume meshes and their nearest-source reference locations differ
from the surface-mirror meshes used by the boundary groups.
