[Mesh]
    type = FoamMesh
    case = 'foam'
    foam_patch = 'left right'
[]

[FoamBCs]
    [T_value]
        type=FoamFixedValueBC
        foam_variable = T
        initial_condition = 0.
        boundary = 'left'
    []
    [T_flux]
        type=FoamDiffusionFluxBC
        foam_variable = T
        boundary = 'right'
    []
[]


[Problem]
    type = FoamProblem
[]

[Executioner]
    type = Transient
    end_time = 32
    [TimeSteppers]
        [foam]
            type = FoamTimeStepper
        []
    []
[]
