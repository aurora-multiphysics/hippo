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
            type = FoamTimeStepper
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
    [t_avg_multiple] # Should be same as t_avg
        type = FoamSideAverageValue
        foam_variable = 'T'
        boundary = 'top bottom'
        execute_on = TIMESTEP_END
    []
    [U_avg_magnitude]
        type = FoamSideAverageValue
        foam_variable = 'U'
        boundary = right
        execute_on = TIMESTEP_END
    []
    [U_avg_magnitude_multiple] # Should be the same as U_avg_multitude
        type = FoamSideAverageValue
        foam_variable = 'U'
        boundary = 'left right'
        execute_on = TIMESTEP_END
    []
    [U_avg_normal]
        type = FoamSideAverageValue
        foam_variable = 'U'
        boundary = right
        component = normal
        execute_on = TIMESTEP_END
    []
    [U_avg_normal_multiple] # Should be zero, left and right should cancel
        type = FoamSideAverageValue
        foam_variable = 'U'
        boundary = 'right left'
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
    [U_avg_x_multiple] # Should be same as U_avg_x
        type = FoamSideAverageValue
        foam_variable = 'U'
        boundary = 'left right'
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
    [U_avg_y_multiple] # Should be same as U_avg_y
        type = FoamSideAverageValue
        foam_variable = 'U'
        boundary = 'left right'
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
    [heat_flux]
        type = FoamSideAverageFunctionObject
        function_object = 'wallHeatFlux'
        boundary = right
        component = x
        execute_on = TIMESTEP_END
    []
    [heat_flux_multiple] # should be zero
        type = FoamSideAverageFunctionObject
        function_object = 'wallHeatFlux'
        boundary = 'left right'
        component = x
        execute_on = TIMESTEP_END
    []
[]

[Outputs]
    exodus = false
    csv = true
[]
