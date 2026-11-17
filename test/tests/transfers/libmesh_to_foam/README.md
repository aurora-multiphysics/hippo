# Native boundary transfer tests

These integration cases check transfer configuration, source-field updates, MPI
participation, and writes into registered native OpenFOAM boundary fields.
Evaluator algorithms (ties, p-refinement, block restrictions, errors, and partition
independence) are covered by the evaluator unit tests.

The `fieldTransferTest` OpenFOAM solver reads and writes `transferField` without
changing it. Both parent and child use a unit-cube mesh. The parent mirrors its
left/right patches into libMesh and assigns the spatially and temporally varying
function in `main.i`. A FunctionIC initializes the source; a FunctionAux updates
it at timestep end before the child executes. The child registers the native
field through `FoamVariableField`, without creating a reference AuxVariable.

| Case | Parent/child ranks | Source | Sampling |
|---|---|---|---|
| Shape serial | 1/1 | CONSTANT MONOMIAL | Default shape evaluation |
| Nearest nodes serial | 1/1 | FIRST LAGRANGE | Explicit nodes, default N=1 |
| Shape MPI | 4/2 | CONSTANT MONOMIAL | Default shape evaluation |
| Nearest centroids MPI | 4/2 | CONSTANT MONOMIAL | Default variable-based sampling, N=2 |

Shape cases use matching 4 x 3 x 2 meshes. Nearest cases retain that parent mesh
but use a 4 x 4 x 3 child mesh, giving 12 target faces per patch instead of six.
Nonmatching face centers distinguish nearest sampling from interpolation. The
centroid case also distinguishes averaging two samples from selecting one.
Parent MPI decomposition is along x; child decomposition is along z. Some parent
ranks do not host the child, and source contributions must cross ranks.

Fluidfoam checks left/right boundary values at times 1, 2, and 3. Nearest reference
locations are constructed analytically from the parent grid, independently of the
transfer implementation. The chosen mesh resolutions avoid distance ties across the
selection cutoff, so the expected values do not depend on entity numbering.

`setup.sh [serial|mpi_4_2] [shape|nearest]` prepares the selected mesh profile.
The harness orders all cases through prerequisites because they reuse
`parent_case` and `child_case`. MPI output is reconstructed before verification.
