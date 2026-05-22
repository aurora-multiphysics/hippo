[Mesh]
    type = FoamMesh
    case = 'foam'
    foam_patch = 'left right'
[]

[Variables]
    [dummy]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 999
    []
[]

[FoamBCs]
    [T_value]
        type=FoamFixedValuePostprocessorBC
        foam_variable = T
        default = 0.
        boundary = 'left'
    []
    [T_flux]
        type=FoamDiffusionFluxPostprocessorBC
        foam_variable = T
        boundary = 'right'
        default=2
    []
[]


[Problem]
    type = FoamProblem
[]

[Executioner]
    type = Transient
    end_time = 1
    [TimeSteppers]
        [foam]
            type = FoamTimeStepper
        []
    []
[]

[Outputs]
    exodus = true
[]
