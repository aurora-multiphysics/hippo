[Mesh]
  type = FoamMesh
  case = 'buoyantCavity'
  foam_patch = 'topAndBottom frontAndBack'
[]

[Variables]
  [Dummy]
    initial_condition = 3.14
  []
[]

[AuxVariables]
  [T]
    initial_condition = 111
    family = MONOMIAL
    order = CONSTANT
  []
[]

[FoamBCs]
  [T]
    type = FoamFixedValueBC
    foam_variable = T
    v = T
  []
[]

[FoamVariables]
  [foam_T]
    type = FoamVariableField
    foam_variable = T
  []
  [foam_hf]
    type = FoamFunctionObject
    foam_variable = wallHeatFlux
  []
[]

[Problem]
  type=FoamProblem
[]

[Executioner]
  type = Transient
  start_time = 1
  end_time = 2.5
  dt = 0.5
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  [TimeStepper]
    type = FoamTimeStepper
  []
[]

[Outputs]
  exodus = true
[]
