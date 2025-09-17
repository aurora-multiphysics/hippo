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
    [temp]
        type=FoamFixedValueBC
        foam_field = T
        v = T_bc
    []
[]

[AuxVariables]
    [T_shadow]
        type = FoamVariableField
        foam_variable = 'T'
    []
    [T_bc]
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
    [T_bc]
        type=ParsedAux
        variable = T_bc
        expression = '0.05 + (x + y + z)*t'
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
