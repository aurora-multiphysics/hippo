[Mesh]
  type = FoamMesh
  foam_args = '-case buoyantCavity'
  foam_patch = 'int_bottom'
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
  [whf]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0
  []
  [wt]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 300
  []
[]

[Problem]
  type=BuoyantFoamProblem
  wall_heat_flux = whf
  wall_temperature = wt
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 1
  dt = 0.01
  [./TimeStepper]
    type = FoamTimeStepper
  [../]
[../]

[Outputs]
  exodus = true
[]
