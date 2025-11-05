[Mesh]
    type = FoamMesh
    case = 'foam'
    foam_patch = 'left right bottom top back front'
[]

[FoamBCs]
    [mass_flow]
        type=FoamMassFlowRateMappedInletBC
        boundary = 'left'
        mass_flow_pp = pp
        translation_vector = '0.5 0 0'
    []
    [temp]
        type=FoamScalarBulkMappedInletBC
        boundary = 'left'
        mass_flow_pp = pp
        translation_vector = '0.5 0 0'
        foam_variable = 'T'
    []
[]

[Postprocessors]
    [pp]
        type = ParsedPostprocessor
        expression = '1'
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
