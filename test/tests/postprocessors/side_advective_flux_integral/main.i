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
    [m_dot_x1]
        type = FoamSideAdvectiveFluxIntegral
        foam_scalar = 'rho'
        block = left
    []
    [m_dot_x2]
        type = FoamSideAdvectiveFluxIntegral
        foam_scalar = 'rho'
        block = right
    []
    [m_dot_y1]
        type = FoamSideAdvectiveFluxIntegral
        foam_scalar = 'rho'
        block = bottom
    []
    [m_dot_y2]
        type = FoamSideAdvectiveFluxIntegral
        foam_scalar = 'rho'
        block = top
    []
    [m_dot_z1]
        type = FoamSideAdvectiveFluxIntegral
        foam_scalar = 'rho'
        block = back
    []
    [m_dot_z2]
        type = FoamSideAdvectiveFluxIntegral
        foam_scalar = 'rho'
        block = front
    []
[]

[Outputs]
    exodus = false
    csv = true
[]
