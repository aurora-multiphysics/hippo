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
        type = FoamSideIntegratedValue
        foam_variable = 'T'
        boundary = top
        execute_on = TIMESTEP_END
    []
    [U_avg_magnitude]
        type = FoamSideIntegratedValue
        foam_variable = 'U'
        boundary = back
        execute_on = TIMESTEP_END
    []
    [U_avg_normal]
        type = FoamSideIntegratedValue
        foam_variable = 'U'
        boundary = bottom
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
    [U_avg_y]
        type = FoamSideIntegratedValue
        foam_variable = 'U'
        boundary = back
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
        type = FoamSideIntegratedValue
        foam_variable = 'wallHeatFlux'
        boundary = right
        component = x
        execute_on = TIMESTEP_END
    []
[]

[Outputs]
    exodus = false
    csv = true
[]
