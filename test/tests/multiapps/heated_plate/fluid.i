[Mesh]
  type = FoamMesh
  foam_args = '-case fluid-openfoam'
  foam_patch = 'interface'
  dim=2
[]

[Variables]
  [FOO]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 1
  []
[]

[AuxVariables]
  [wall_heat_flux]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0
  []
  [wall_temperature]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 300
  []
[]

[Problem]
  type=BuoyantFoamProblem
  wall_heat_flux = wall_heat_flux
  wall_temperature = wall_temperature
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 5
  dt = 0.05
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
