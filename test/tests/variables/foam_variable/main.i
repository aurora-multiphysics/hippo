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
