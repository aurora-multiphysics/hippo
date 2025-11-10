[Mesh]
  type = FoamMesh
  case = 'foaminput'
  foam_patch = ' Wall-2 Wall-3 Wall-4 Wall-5'
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
  type = FoamProblem
  solve = false
[]
[Executioner]
  type = Transient
  start_time = 0.0
  end_time = 0.1
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  [./TimeStepper]
    type = FoamTimeStepper
  [../]
[../]

[Outputs]
  exodus = true
[]
