# Native boundary transfer tests

All cases run a Hippo parent and child with identical 4 × 3 × 2 cube meshes.
The `fieldTransferTest` OpenFOAM test solver reads and writes `transferField` but
never changes it. The parent source is a CONSTANT-order MONOMIAL AuxVariable on the left/right mirror.
A FunctionIC supplies initial data and a FunctionAux refreshes it at timestep end,
before the child MultiApp executes. The function varies in space and time to ensure the transfer
from libMesh to OpenFOAM occurs correctly.
`transferField` is initialised to -99 to ensure that boundary patch values are correctly overridden.

The cases are run in:

- Serial
- Parallel with two ranks each with different partitioning to ensure remote source contributions are required.
- Parallel with 4 ranks on the parent and 2 on the child to check transfers work where some parent ranks don't host the child.

The destination `FoamVariableField` registers references to the boundary fields of the native `transferField` volScalarField.

Fluidfoam reads the destination boundary values at each output timestep and compares them against the prescribed function.
