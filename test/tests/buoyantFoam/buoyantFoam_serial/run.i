[Mesh]
  type = FoamMesh
  foam_args = '-case buoyantCavity'
  foam_patch = 'topAndBottom frontAndBack'
  dim=2
[]
[AuxVariables]
  [T]
    initial_condition = 1.0
  []
[]
[Variables]
  [FOO]
    initial_condition = 10.0
  []
[]
[Problem]
  type=BuoyantFoamProblem
[]
[Executioner]
  type = Transient
  start_time = 0
  end_time = 11
  dt = 0.1
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
