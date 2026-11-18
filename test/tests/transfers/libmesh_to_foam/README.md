# Native boundary transfer tests

`main.i` contains two transfers targeting the same `received` object and native
`transferField`. Only one is active per run:

- `Transfers/active=to_foam` selects explicit left/right boundaries (the default).
- `Transfers/active=to_foam_default` omits `foam_boundaries` and selects every
  physical patch exposed by the accessor.

Separate runs and freshly prepared cases prevent either transfer from masking a
failure in the other. The complete setup remains in `main.i`.

The parent mirrors all six physical patches. The sampling-test source is restricted
to left/right, preserving its analytical nearest-node and nearest-centroid
expectations. Default-selection runs override the same source to FIRST LAGRANGE
on all six physical patches. Shape interpolation reproduces the prescribed spatially linear function
on both matching and nonmatching destination meshes. A FunctionIC initializes the source; a FunctionAux updates it at timestep end.

The `fieldTransferTest` solver reads and writes `transferField` without changing
its values. The receiving FoamVariableField creates an AuxVariable, covering
registration with mirroring enabled.

| Case | Parent/child ranks | Explicit transfer source | Explicit sampling |
|---|---|---|---|
| Shape serial | 1/1 | CONSTANT MONOMIAL | Shape evaluation |
| Nearest nodes serial | 1/1 | FIRST LAGRANGE | Nodes, N=1 |
| Shape MPI | 4/2 | CONSTANT MONOMIAL | Shape evaluation |
| Nearest centroids MPI | 4/2 | CONSTANT MONOMIAL | Variable default, N=2 |

Shape cases use matching 4 x 3 x 2 meshes. Nearest cases use a 4 x 4 x 3 child
mesh. Parent decomposition is along x; child decomposition is along z. Two parent
ranks do not host the child in MPI runs. Default selection must exclude processor
patches: their interior face centres are outside the source surface mesh.

Python checks values at times 1, 2, and 3. Explicit-selection runs verify
left/right against independently constructed source samples. Separate default-
selection runs in serial and MPI 4/2 verify all six physical patches against the
prescribed function, plus unchanged internal values. The nearest sample
geometries avoid ties across the selection cutoff; detailed evaluator behavior
is covered by unit tests.

The empty-boundary case uses the same `main.i`, activates only `to_foam`, and sets
its boundary list explicitly empty through CLI arguments. All boundary and
internal values of `transferField` must remain at -99. There is no unsupported-
volume test.

`setup.sh [serial|mpi_4_2] [shape|nearest]` prepares the cases. Prerequisites keep
runs and verification sequential because they share `parent_case` and
`child_case`. The receiving field is reconstructed before MPI verification.
