[Mesh]
    type = FoamMesh
    case = 'foam'
    foam_patch = 'left right bottom top back front'
[]

[Variables]
    [dummy]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 999
    []
[]

[AuxVariables]
    [fluid_wall_temp]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0.0
    []
    [heat_flux]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0.0
    []
[]

[AuxKernels]
    [fluid_temp]
        type = ParsedAux
        variable = fluid_wall_temp
        expression = 't'
        use_xyzt = true
    []
[]

[Problem]
    type = FoamProblem
    # Take the boundary temperature from OpenFOAM and set it on the MOOSE mesh.
    temp = fluid_wall_temp
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
