[Mesh]
    type = FoamMesh
    case = 'foam'
    foam_patch = 'left right top front bottom back'
[]

[Variables]
    [dummy]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 999
    []
[]

[FoamVariables]
    [T_shadow]
        type = FoamWallTemperature
        boundary = 'left right top front bottom back'
    []
    [whf_shadow]
        type = FoamWallHeatFlux
        boundary = 'left right top front bottom back'
    []
[]

[Problem]
    type = FoamProblem
[]

[Executioner]
    type = Transient
    end_time = 0.32
    [TimeSteppers]
        [foam]
            type = FoamTimeStepper
        []
    []
[]

[Outputs]
    exodus = true
[]
