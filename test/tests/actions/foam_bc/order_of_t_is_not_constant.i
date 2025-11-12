[Mesh]
  type = FoamMesh
  case = './foam'
  foam_patch = 'left right'
[]

[FoamBCs]
  [T]
    type = FoamFixedValueBC
    foam_variable = T
    v = T
  []
[]

[AuxVariables]
  [T]
    initial_condition = 111
    family = MONOMIAL
    order = CONSTANT
  []
  [foam_T]
    initial_condition = 999
    order = CONSTANT
    family = MONOMIAL
  []
  [foam_hf]
    initial_condition = 999
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Problem]
  type=FoamProblem
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 1
  dt = 1

  [TimeStepper]
    type = FoamTimeStepper
  []
[]
