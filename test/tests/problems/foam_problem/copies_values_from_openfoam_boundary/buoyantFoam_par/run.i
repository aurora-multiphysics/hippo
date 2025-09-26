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

[FoamVariables]
  [T]
    type = FoamVariableField
    foam_variable = T
  []
  [hf]
    type = FoamFunctionObject
    foam_variable = wallHeatFlux
  []
[]

[FoamBCs]
  [T]
    type = FoamFixedValueBC
    foam_variable = T
    v = T
  []
[]

[Problem]
  type=FoamProblem
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 20
  dt = 1
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
