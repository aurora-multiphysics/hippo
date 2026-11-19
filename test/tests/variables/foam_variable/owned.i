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
  [boundary_only]
    type = FoamScalarVariable
    boundary = left
    initial_condition = 7
    create_aux_variable = true
  []
  [internal_only]
    type = FoamScalarVariable
    volume = true
    initial_condition = 3
  []
  [combined]
    type = FoamScalarVariable
    boundary = 'left right'
    volume = true
    initial_condition = 5
  []
  [vector_storage]
    type = FoamVectorVariable
    boundary = right
    volume = true
    initial_condition = '1 2 3'
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
            type = FoamTimeStepper
        []
    []
[]
