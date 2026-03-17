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

[FoamBCs]
    [temp1]
        type=FoamFixedValuePostprocessorBC
        foam_variable = T
        boundary = 'left right top' # test boundary restrictions
        pp = T_bc1
    []
    [temp2]
        type=FoamFixedValuePostprocessorBC
        foam_variable = T
        boundary = 'bottom front back' # test boundary restrictions
        pp = T_bc2
    []
[]

[FoamVariables]
    [T_shadow]
        type = FoamVariableField
        foam_variable = 'T'
    []
    [e_shadow]
        type = FoamVariableField
        foam_variable = 'e'
    []
    [whf_shadow]
        type = FoamFunctionObject
        foam_variable = 'wallHeatFlux'
    []
[]

[Postprocessors]
    [T_bc1]
        type=ParsedPostprocessor
        expression = '0.05 + t'
        use_t = true
        execute_on='TIMESTEP_BEGIN INITIAL'
    []
    [T_bc2]
        type=ParsedPostprocessor
        expression = '0.05 + 2*t'
        use_t = true
        execute_on='TIMESTEP_BEGIN INITIAL'
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
[]
