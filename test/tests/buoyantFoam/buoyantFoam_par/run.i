[Mesh]
  type = FoamMesh
  foam_args = '-case buoyantCavity -parallel'
  foam_patch = 'frontAndBack topAndBottom'
  dim=2
[]
[AuxVariables]
  [T]
  initial_condition = 1.0
  []
[]
[Variables]
[FOO]
initial_conditions
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
