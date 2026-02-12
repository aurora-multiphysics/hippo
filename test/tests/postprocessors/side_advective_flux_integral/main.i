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
            type = FoamTimeStepper
        []
    []
[]

[Postprocessors]
    [m_dot_x1]
        type = FoamSideAdvectiveFluxIntegral
        foam_scalar = 'rho'
        boundary = left
    []
    [m_dot_x2]
        type = FoamSideAdvectiveFluxIntegral
        foam_scalar = 'rho'
        boundary = right
    []
    [m_dot_x12] # should be zero
        type = FoamSideAdvectiveFluxIntegral
        foam_scalar = 'rho'
        boundary = 'left right'
    []
    [m_dot_y1]
        type = FoamSideAdvectiveFluxIntegral
        foam_scalar = 'rho'
        boundary = bottom
    []
    [m_dot_y2]
        type = FoamSideAdvectiveFluxIntegral
        foam_scalar = 'rho'
        boundary = top
    []
    [m_dot_y12] # should be zero
        type = FoamSideAdvectiveFluxIntegral
        foam_scalar = 'rho'
        boundary = 'bottom top'
    []
    [m_dot_z1]
        type = FoamSideAdvectiveFluxIntegral
        foam_scalar = 'rho'
        boundary = back
    []
    [m_dot_z2]
        type = FoamSideAdvectiveFluxIntegral
        foam_scalar = 'rho'
        boundary = front
    []
    [m_dot_z12] # should be zero
        type = FoamSideAdvectiveFluxIntegral
        foam_scalar = 'rho'
        boundary = 'front back'
    []
[]

[Outputs]
    exodus = false
    csv = true
[]
