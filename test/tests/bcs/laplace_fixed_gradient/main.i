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
    [temp1]
        type=FoamFixedValueBC
        foam_variable = T
        v = T_value
        boundary = 'left'
    []
    [temp2]
        type=FoamFixedGradientBC
        foam_variable = T
        v = T_flux
        boundary = 'right'
        diffusivity_coefficient = kappa
    []
[]

[AuxVariables]
    [T_value]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0.
    []
    [T_flux]
        family = MONOMIAL
        order = CONSTANT
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
