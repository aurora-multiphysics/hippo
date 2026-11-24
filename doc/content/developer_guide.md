# Developer Guide

This page contains information to aid in development of Hippo.

## OpenFOAM field access

`FoamFieldAccessor<T>` in `include/base` borrows field storage and exposes
`internalField()` and `boundaryField(patch)` as `Foam::Field<T>&`. Internal and
boundary storage are independently optional; accessing unavailable storage reports
an error. Constructors accept an internal field alone, a boundary-value map with
optional internal storage, or a native `Foam::VolField<T>` exposing both its
internal values and non-processor boundary values. An empty accessor is also valid.
References are fixed at construction, while field values remain writable.

Partial accessors expose only the supplied patches and storage. A boundary
reference can expose native values, gradient arrays, or provider-owned buffers.
The accessor does not convert physical quantities: gradient storage still holds
gradients, and any heat-flux conversion belongs to the provider.

`FoamProblem::getFieldAccessor<T>(name)` resolves an accessor by the exposing
object's qualified name, such as `FoamVariables/T` or `FoamBCs/T`. Lookup uses
the exact registered key, short names are not resolved. `registerField(name, accessor)` takes ownership
of the accessor under the supplied qualified name. Duplicate qualified names
and mismatched field types report errors.

`registerVolFieldVariable<T>(variable, field)` creates a separate accessor under
`FoamVariables/variable`, borrowing the named OpenFOAM volume field. Multiple objects can wrap
the same volume field: their accessor reference maps are separate, but values
remain shared. Native field names are not automatically registered or reserved.
For example, `temperature` may wrap native `T` while a standalone object named
`T` exposes unrelated buffers; transfers select the registered object name.

The existing transfer parameter `foam_variable` accepts any registered object
qualified name, including storage supplied by a BC. BCs and future sources must explicitly
register the storage they expose. Registering an accessor does not automatically
expose existing BC storage.

`FoamScalarVariable` and `FoamVectorVariable` share the templated `FoamVariable<T>`
implementation. Supplying `foam_variable` wraps an existing OpenFOAM volume field;
otherwise `boundary` selects owned patch buffers and `volume = true` allocates
owned internal storage. Allocation parameters cannot be combined with `foam_variable`.
Owned storage uses `initial_condition` (zero by default).

`create_aux_variable` controls boundary-only scalar mirroring. It defaults to true
for wrapped scalar fields and false for owned fields and vectors. Vector mirroring
is rejected. Mirroring is restricted to exposed boundaries present in FoamMesh;
internal storage does not imply volumetric MOOSE mirroring.

Owned buffers register during construction; wrapped fields register during
`initialSetup()`, before INITIAL transfers. Derived variables can update values through the protected
`getFieldAccessor()` method, then call the base `syncVariable()` to mirror them. `FoamWallVariableBase` uses this path to store calculated wall quantities
in owned boundary buffers, then mirror them by default. Disabling mirroring still
updates those buffers during solution synchronization; accessor reads return the
most recently calculated values. Wall variables do not wrap existing fields or
allocate internal storage. OpenFOAM owns wrapped storage; the variable owns allocated buffers. Both
must keep referenced field objects stable for the accessor's lifetime.

## libMesh-to-OpenFOAM source evaluation

`MultiApplibMeshToFoamTransfer` supports parent-to-child transfers with exactly one
child instance in the selected `to_multi_app`. Other directions or instance counts
produce an error. The parent may contain other named MultiApps independently.
The destination must provide a `FoamProblem` and the configured field accessor
and patches. The child instance, its rank allocation, and its field accessor
must remain fixed after setup.

When `foam_boundaries` is omitted, transfers use all boundaries exposed by the
receiving accessor. An explicit list selects only those patches, and an explicitly
empty list transfers no boundary values. A partial accessor therefore defaults to
its selected patches; a full volume-field accessor excludes processor patches. Default patch names are combined across destination ranks so
all parent ranks participate in the same sequence of collective evaluations.

`volume` defaults to `false`. Setting it to `true` transfers values evaluated at
OpenFOAM cell centres into the receiving accessor's internal field. This field
must exist and have one entry per local cell. Missing storage and size mismatches
are reported collectively, including on parent ranks without a child.

For volume-only transfers, use:

```text
volume = true
foam_boundaries = ''
```

For combined transfers, set `volume = true` and omit `foam_boundaries` to select
all exposed physical boundaries, or provide an explicit boundary list. Volume is
evaluated first, followed by the common boundary order. No boundary-condition
correction is performed after writing. An empty boundary selection with
`volume = false` remains a valid no-op.

Cell and face centres are mapped into the source coordinate frame and cached at
setup. Geometry, topology, ownership, and receiving storage must remain fixed;
source values are read again on every execution. Shape evaluation requires source
elements covering the target cell centres; a boundary-only source mesh generally
cannot provide volume values. Nearest-location evaluation retains its existing
extrapolation behavior. MOOSE auxiliary mirroring remains boundary-only.

`MultiApplibMeshToFoamTransfer` uses `evaluation_method = shape_function` by default.
To transfer the globally nearest source sample instead, set:

```text
evaluation_method = nearest_location
source_type = centroids
```

`source_type` defaults to `variable_default` for nearest-location evaluation and
cannot be explicitly set for shape-function evaluation. Choose `nodes` for first- or second-order Lagrange nodal
samples, or `centroids` for samples at element vertex averages. All centroid samples use element-local FE evaluation at the vertex average,
including constant fields and p-refined elements. Node sampling rejects p-refined
source meshes; choose `source_type = centroids` for those meshes.
The default `source_type = variable_default` uses nodes for nodal variables and
vertex averages for elemental variables, following MOOSE's variable-based selection. Node sampling, whether explicit or selected by
`variable_default`, rejects discontinuous fields and unsupported nodal FE types.

Nearest-location evaluation averages the globally nearest `num_nearest_points` samples
(default 1), with equal weight regardless of distance. If fewer samples exist globally,
all available samples are averaged. Exact distance ties are resolved by global entity ID.
With stable geometry, field values, and entity IDs, selection and summation order are
independent of source rank ownership. Each rank sends at most N owned samples; ghost
copies do not contribute.

This retains MOOSE's fixed-count averaging concept but deliberately differs from its
per-rank averaging and lowest-rank tie-break. MOOSE's optional `search_value_conflicts`
diagnostics are not implemented here. Targets outside the source mesh receive a
nearest-location value; no eligible sample produces an error.

The evaluator builds its source KD-tree once and reads the current solution only
for the selected local samples in each query. These
reads are local; the source solution must already be synchronized before transfer
evaluation. Source geometry, topology, ownership, variable support,
destination geometry, and app transforms must remain fixed. Displaced sample
positions are supported when fixed; moving-mesh cache invalidation is not implemented.

## Evaluator unit tests

The evaluator tests construct real libMesh meshes and equation systems; no OpenFOAM
case or MOOSE input file is needed. Both evaluators also expose direct libMesh
constructors for these fixtures, while their MOOSE constructors retain variable
lookup and displaced-mesh selection.

After building the unit executable, run from the repository root:

```bash
./unit/run_tests --gtest_filter='LibMeshEvaluatorTest.*'
mpiexec -n 2 ./unit/run_tests --gtest_filter='LibMeshEvaluatorMPITest.*'
```

`METHOD` selects the executable variant and defaults to `opt`. The runner forwards
GoogleTest arguments; with no arguments it runs all tests. MPI evaluator tests skip
unless there are exactly two ranks. Serial fixtures use a self communicator, so
they remain independent when the complete executable runs under MPI.

Coverage includes an interior sampling line crossing element and partition
interfaces, discontinuous averages, source-block restrictions, nearest-location
ties, solution updates, fixed displaced geometry, and collective error handling.
MPI fixtures explicitly assign ownership and repeat cases with swapped ownership.
Tests can include `MPITestFlags.h` and put a run restriction first in the test body:

- `REQUIRE_MPI()` skips single-rank runs.
- `REQUIRE_MPI_RANKS(2)` requires exactly two ranks (used by the MPI evaluator cases).
- `SKIP_WITH_MPI()` skips runs with more than one rank.
- No flag runs in either mode.

These checks use the world communicator, regardless of the communicator used by
an individual fixture. They report GoogleTest skips and must precede collective
work. Flags do not launch MPI; use `mpiexec` to select the number of processes.

Builds and test runs are intentionally left to the developer for this change.

## Documentation

The project uses the MOOSE docs system to generate its HTML documentation.
The HTML artifacts are generated and deployed within Hippo's CI.
The CI pipeline is run on any pull request
and also on a push to `main` (i.e., a merged pull request).
The documentation pages are always built,
however they are only deployed on a push to `main`.

## Apptainer

The [Apptainer](https://apptainer.org/docs/user/main/) containers are used to test Hippo within the CI pipeline.
There are two containers
- **Development container:** this contains all of Hippo's dependencies but not Hippo itself.
The definition file can be found [here](https://github.com/aurora-multiphysics/hippo/apptainer/hippo-dev.def).
- **Release container:** used to build and test Hippo within the CI environment.
The definition file can be found [here](https://github.com/aurora-multiphysics/hippo/apptainer/hippo-release.def).

The development container is created on
[workflow dispatch](https://github.com/aurora-multiphysics/hippo/actions/workflows/deploy-apptainer-dev.yaml)
and stored under
[UKAEA's Quay.io organisation](https://quay.io/repository/ukaea/hippo)
with name `hippo:dev-` followed by the first 10 characters of the Git revision that created it.
Once the image has been built and pushed to Quay.io,
a developer should open a PR that updates `.github/workflows/ci.yml`
A [robot account](https://docs.quay.io/glossary/robot-accounts.html)
has been set up with write permissions for the Hippo repository.
This robot account is used to authenticate between Quay and GitHub.
Contact UKAEA's Quay.io admins (or the RSE team) for support with this account.

## Direct boundary-condition transfers

`FoamFixedValueBC`, `FoamFixedGradientBC` and `FoamDiffusionFluxBC` default to
`direct = true`. They register a boundary-only accessor under the BC object name.
A libMesh-to-Foam transfer targets that name through its `foam_variable` parameter,
without creating an AuxVariable or a separate FoamVariable wrapper.

The writable values are the prescribed patch values, normal gradients or diffusion
fluxes respectively. Explicit BC boundaries can be physical OpenFOAM patches outside
the mirrored FoamMesh. Omitting the BC boundary list retains the mirrored-boundary
default. Transfer boundary selection follows the usual accessor rules.

Set `direct = false` for the existing MOOSE-variable input path. In that mode, `v`
selects an existing CONSTANT MONOMIAL variable, or an AuxVariable is created with the
BC object's name. Supplying `v` in direct mode is an error. Existing variable-driven
inputs must explicitly select `direct = false`. A BC and FoamVariable can share
an object name, transfers select `foam_variable = FoamBCs/T` or
`foam_variable = FoamVariables/T`. AuxVariable names must still be distinct,
use `v` to select an existing input variable when needed.
Postprocessor-driven BCs retain their existing input behaviour.

Patch replacement finishes before accessors register and before INITIAL transfers.
Direct BC imposition leaves transferred data untouched, with patch evaluation and
coefficient updates handled by OpenFOAM. Explicit `initial_condition` initializes the prescribed scalar
storage before transfers, otherwise compatible patch data is retained and newly
created storage starts at zero.

`FoamDiffusionFluxBC` uses the `hippoDiffusionFlux` OpenFOAM patch in `include/foam`
and `src/foam`. Its writable `flux()` field stores the prescribed flux, positive
into the domain. `updateCoeffs()` computes `gradient = flux / diffusivity` from the
current named volume field (`kappa` by default), rejecting zero diffusivity.
For temperature and thermal conductivity, flux is measured in W/m². The patch writes
`flux`, `diffusivity` and `fluxDimensions` for restart, along with its gradient. Like `fixedGradient`, it omits the boundary
value and reconstructs it from the internal field and saved gradient when reading.
Storing dimensions explicitly lets the patch read flux before the solver
registers its diffusivity field. Clone, mapping and reset operations preserve flux.
