# FoamTimeStepper


!syntax description /Executioner/TimeSteppers/FoamTimeStepper

`FoamTimeStepper`'s prime responsibility is to synchronise MOOSE and OpenFOAM,
providing MOOSE with OpenFOAM's desired time step
and forcing OpenFOAM to take additional steps when MOOSE desires synchronisation.
`FoamTimeStepper` operates in two modes

1. +Passive mode:+ OpenFOAM uses the values in the case's `controlDict` to set the time step.
   e.g. using CFL-adaptive time-stepping if specified. We recommend using this approach.
1. +Active mode:+ `dt`, `end_time`, `start_time` specified,
   which overrides the `controlDict` with new constant values for `dt`.

## Example Input File Syntax

To use `FoamTimeStepper` in passive mode

```
[Executioner]
    ...
    [TimeStepper]
        [foam_dt]
            type = FoamTimeStepper
        []
    []
[]
```

To use it in active mode

```
[Executioner]
    dt = 0.001
    end_time = 10.
    ...
    [TimeStepper]
        [foam_dt]
            type = FoamTimeStepper
        []
    []
[]
```

!syntax parameters /Executioner/TimeSteppers/FoamTimeStepper

!syntax inputs /Executioner/TimeSteppers/FoamTimeStepper

!syntax children /Executioner/TimeSteppers/FoamTimeStepper
