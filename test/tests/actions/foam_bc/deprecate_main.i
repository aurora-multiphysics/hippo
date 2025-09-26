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

[AuxVariables]
    [T_value]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0.
    []
[]

[Problem]
    type = FoamProblem
    solve = false
    temp = T_value
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
