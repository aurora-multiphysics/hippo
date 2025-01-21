[Mesh]
  type = FoamMesh
  case = 'foaminput'
  foam_patch = ' Wall-2 Wall-3 Wall-4 Wall-5'
[]
[Variables]
  [T]
    initial_condition = 1.0
    family = MONOMIAL
    order = CONSTANT
  []
  [hf]
    initial_condition = 1.0
    family = MONOMIAL
    order = CONSTANT
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
