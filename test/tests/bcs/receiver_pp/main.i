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
        type=FoamFixedGradientPostprocessorBC
        foam_variable = T
        boundary = 'right'
        diffusivity_coefficient = kappa
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
            type = FoamControlledTimeStepper
        []
    []
[]

[Outputs]
    exodus = true
[]
