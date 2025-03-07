[Mesh]
  type = FoamMesh
  case = 'foaminput'
  foam_patch = ' Wall-1 Wall-3'
[]
[Variables]
  [T]
  initial_condition = 1.0
  []
  [hf]
  initial_condition = 1.0
  []
[]
[Problem]
  type = FoamProblem
  solve = false
  temp = T
  foam_temp = T
  foam_heat_flux = hf
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
