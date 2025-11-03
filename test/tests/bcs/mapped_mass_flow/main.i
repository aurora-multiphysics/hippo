[Mesh]
    type = FoamMesh
    case = 'foam'
    foam_patch = 'left right bottom top back front'
[]

[FoamBCs]
    [temp1]
        type=FoamMassFlowRateMappedInletBC
        boundary = 'left' # test boundary restrictions
        mass_flow_pp = pp
        translation_vector = '0.5 0 0'
    []
[]

[Postprocessors]
    [pp]
        type = ParsedPostprocessor
        expression = 't'
        use_t = true
        execute_on = TIMESTEP_BEGIN
    []
[]


[Problem]
    type = FoamProblem
    # Take the boundary temperature from OpenFOAM and set it on the MOOSE mesh.
[]

[Executioner]
    type = Transient
    end_time = 5
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
