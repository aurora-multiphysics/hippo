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
        type = FoamSideIntegratedValue
        foam_variable = 'T'
        boundary = top
        execute_on = TIMESTEP_END
    []
    [t_avg_multiple] # should be double t_avg
        type = FoamSideIntegratedValue
        foam_variable = 'T'
        boundary = 'bottom top'
        execute_on = TIMESTEP_END
    []
    [U_avg_magnitude]
        type = FoamSideIntegratedValue
        foam_variable = 'U'
        boundary = back
        execute_on = TIMESTEP_END
    []
    [U_avg_magnitude_multiple] # should be double U_avg_magnitude
        type = FoamSideIntegratedValue
        foam_variable = 'U'
        boundary = 'back front'
        execute_on = TIMESTEP_END
    []
    [U_avg_normal]
        type = FoamSideIntegratedValue
        foam_variable = 'U'
        boundary = bottom
        component = normal
        execute_on = TIMESTEP_END
    []
    [U_avg_normal_multiple] # should be zero
        type = FoamSideIntegratedValue
        foam_variable = 'U'
        boundary = 'bottom top'
        component = normal
        execute_on = TIMESTEP_END
    []
    [U_avg_x]
        type = FoamSideIntegratedValue
        foam_variable = 'U'
        boundary = front
        component = x
        execute_on = TIMESTEP_END
    []
    [U_avg_x_multiple] # should be double U_avg_x
        type = FoamSideIntegratedValue
        foam_variable = 'U'
        boundary = 'front back'
        component = x
        execute_on = TIMESTEP_END
    []
    [U_avg_y]
        type = FoamSideIntegratedValue
        foam_variable = 'U'
        boundary = back
        component = y
        execute_on = TIMESTEP_END
    []
    [U_avg_y_multiple] # should be double U_avg_y
        type = FoamSideIntegratedValue
        foam_variable = 'U'
        boundary = 'back front'
        component = y
        execute_on = TIMESTEP_END
    []
    [U_avg_z]
        type = FoamSideIntegratedValue
        foam_variable = 'U'
        boundary = front
        component = z
        execute_on = TIMESTEP_END
    []
    [heat_flux]
        type = FoamSideIntegratedFunctionObject
        function_object = 'wallHeatFlux'
        boundary = right
        component = x
        execute_on = TIMESTEP_END
    []
    [heat_flux_multiple] # should be zero
        type = FoamSideIntegratedFunctionObject
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
