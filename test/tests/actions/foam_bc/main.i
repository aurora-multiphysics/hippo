[Mesh]
    type = FoamMesh
    case = 'foam'
    foam_patch = 'left right'
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
        type=FoamTestBC
        foam_variable = T
        v = T_value
        boundary = 'left'
    []
    [temp_r]
        type=FoamTestBC
        foam_variable = T
        v = T_value
        boundary = 'right'
    []
[]

[AuxVariables]
    [T_value]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0.
    []
[]

[Problem]
    type = FoamProblem
[]

[Executioner]
    type = Transient
    end_time = 1
    [TimeSteppers]
        [foam]
            type = FoamControlledTimeStepper
        []
    []
[]

[Outputs]
    exodus = true
[]
