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
        type=FoamFixedValueBC
        foam_field = T
        v = T_bc1
        boundary = 'left right top' # test boundary restrictions
    []
    [temp2]
        type=FoamFixedValueBC
        foam_field = T
        v = T_bc2
        boundary = 'bottom front back' # test boundary restrictions
    []
[]

[AuxVariables]
    [T_shadow]
        type = FoamVariableField
        foam_variable = 'T'
    []
    [T_bc1]
        family = MONOMIAL
        order = CONSTANT
    []
    [T_bc2]
        family = MONOMIAL
        order = CONSTANT
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

[AuxKernels]
    [T_bc1]
        type=ParsedAux
        variable = T_bc1
        expression = '0.05 + (x + y + z)*t'
        use_xyzt = true
        execute_on='TIMESTEP_BEGIN INITIAL'
    []
    [T_bc2]
        type=ParsedAux
        variable = T_bc2
        expression = '0.05 + 2*(x + y + z)*t'
        use_xyzt = true
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
