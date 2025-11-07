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
        pp = T_flux
    []
[]

[Postprocessors]
    [T_flux]
        type = ParsedPostprocessor
        expression = '2*t'
        use_t = true
        execute_on = 'INITIAL TIMESTEP_BEGIN'
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
            type = FoamControlledTimeStepper
        []
    []
[]

[Outputs]
    exodus = true
[]
