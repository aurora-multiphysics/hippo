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

[Problem]
    type = FoamProblem
[]

[Executioner]
    type = Transient
    end_time = 0.32
    [TimeSteppers]
        [foam]
            type = FoamControlledTimeStepper
        []
    []
[]

[Postprocessors]
    [t_avg]
        type = FoamSideAverageValue
        foam_variable = 'T'
        block = top
        execute_on = TIMESTEP_END
    []
    [U_avg_magnitude]
        type = FoamSideAverageValue
        foam_variable = 'U'
        block = right
        execute_on = TIMESTEP_END
    []
    [U_avg_normal]
        type = FoamSideAverageValue
        foam_variable = 'U'
        block = right
        component = normal
        execute_on = TIMESTEP_END
    []
    [U_avg_x]
        type = FoamSideAverageValue
        foam_variable = 'U'
        block = right
        component = x
        execute_on = TIMESTEP_END
    []
    [U_avg_y]
        type = FoamSideAverageValue
        foam_variable = 'U'
        block = right
        component = y
        execute_on = TIMESTEP_END
    []
    [U_avg_z]
        type = FoamSideAverageValue
        foam_variable = 'U'
        block = right
        component = z
        execute_on = TIMESTEP_END
    []
[]

[Outputs]
    exodus = false
    csv = true
[]
