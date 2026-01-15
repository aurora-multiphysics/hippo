[Mesh]
    type = FoamMesh
    case = 'foam'
    foam_patch = 'left'
[]

[Variables]
    [dummy]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 999
    []
[]

[FoamBCs]
    [fluid_wall_temp]
        type = FoamFixedValueBC
        foam_variable = 'T'
        initial_condition = 0.075
    []
[]

[AuxKernels]
    [fluid_temp]
        type = ParsedAux
        variable = fluid_wall_temp
        expression = '1000*t'
        use_xyzt = true
    []
[]

[Problem]
    type = FoamProblem
[]

[Executioner]
    type = Transient
    start_time = 0
    end_time = 0.01
    dt = 0.0003125

    [TimeSteppers]
        [foam]
            type = FoamTimeStepper
        []
    []
[]

[Outputs]
    exodus = true
[]
