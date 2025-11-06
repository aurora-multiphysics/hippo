[Mesh]
    type = FoamMesh
    case = 'foam'
    foam_patch = 'left right bottom top back front'
[]

[FoamBCs]
    [temp1]
        type=FoamMassFlowRateInletBC
        boundary = 'left' # test boundary restrictions
        pp = pp
    []
[]

[Postprocessors]
    [pp]
        type = ParsedPostprocessor
        expression = 't'
        use_t = true
        execute_on = TIMESTEP_BEGIN
    []
    [m_dot]
        type = FoamSideAdvectiveFluxIntegral
        block = left
        foam_scalar = rho
    []
[]


[Problem]
    type = FoamProblem
    # Take the boundary temperature from OpenFOAM and set it on the MOOSE mesh.
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

[Outputs]
    exodus = true
    csv=true
[]
