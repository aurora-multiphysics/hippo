# Energy Conserved in Simple Conduction Example

A cold fluid block sitting next to a warm solid block.
All boundaries are zero gradient,
so no energy should be leaving or entering the system.

```txt
     _____________
    |      |      |  ↑
    | 300K | 310K | 1cm
    |______|______|  ↓
    <-1cm-> <-1cm->
```

We calculate the change in energy
in each block at the initial and final time-step,
and assert that the energy deltas are equal.
