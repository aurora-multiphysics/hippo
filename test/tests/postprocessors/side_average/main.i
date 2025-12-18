[Mesh]
    type = FoamMesh
    case = 'foam'
    foam_patch = 'right top'
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
        boundary = top
        execute_on = TIMESTEP_END
    []
    [U_avg_magnitude]
        type = FoamSideAverageValue
        foam_variable = 'U'
        boundary = right
        execute_on = TIMESTEP_END
    []
    [U_avg_normal]
        type = FoamSideAverageValue
        foam_variable = 'U'
        boundary = right
        component = normal
        execute_on = TIMESTEP_END
    []
    [U_avg_x]
        type = FoamSideAverageValue
        foam_variable = 'U'
        boundary = right
        component = x
        execute_on = TIMESTEP_END
    []
    [U_avg_y]
        type = FoamSideAverageValue
        foam_variable = 'U'
        boundary = right
        component = y
        execute_on = TIMESTEP_END
    []
    [U_avg_z]
        type = FoamSideAverageValue
        foam_variable = 'U'
        boundary = right
        component = z
        execute_on = TIMESTEP_END
    []
[]

[Outputs]
    exodus = false
    csv = true
[]
