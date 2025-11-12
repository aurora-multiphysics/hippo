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
        type=FoamFixedValueBC
        foam_variable = T
        initial_condition = 0.
        boundary = 'left'
    []
    [T_flux]
        type=FoamFixedGradientBC
        foam_variable = T
        boundary = 'right'
        diffusivity_coefficient = kappa
    []
[]

[AuxKernels]
    [T_flux]
        type = ParsedAux
        variable = T_flux
        expression = '2*t'
        use_xyzt = true
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
