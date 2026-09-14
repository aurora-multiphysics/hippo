# Multiphase external-temperature regression

Inputs, Bash setup and Python verification live here. OpenFOAM files use three
working directories: `reference/`, `foam/` and `nonphase/`.

The stock wall-boiling run writes to `reference/`. The coupled HIPPO run uses
`foam/` and is immediately compared with the reference temperatures, mixed
coefficients and evaporation (relative tolerance 1e-7). The comparison also
requires active boiling and checks the imposed flux; no baseline snapshot or
separate reference-verification step is needed.

After coupled verification, setup clears the output times in `foam/` and restores
initial fields before applying zero-gradient liquid and fixed-value gas wall
patches. The spatial run reuses this mesh and directory, exercising temperature
and energy conversion with nonuniform time-dependent flux and relaxation 0.5.
Verification checks exact per-face flux over two steps, direct initialization,
subsequent relaxation and finite temperatures. Test prerequisites enforce this
sequence so coupled results are checked before they are removed.

`nonphase/` independently checks the missing-phaseSystem error. `prep.sh` uses
the configured OpenFOAM environment and creates the reference mesh once, copying
its initial fields, mesh and settings for the coupled run. Tutorial thermophysical
tables are unpacked locally to avoid OpenFOAM/libMesh gzstream conflicts.

Duplicate assignments are tested in `actions/foam_bc`. Parallel, restart and
integrated wall-flux checks remain deferred.
