[Mesh]
  type = FoamMesh
  foam_args = '-case buoyantCavity'
  foam_patch = 'bottom'
  dim = 2
[]

[Variables]
  [dummy]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 999
  []
[]

[AuxVariables]
  [fluid_wall_temp]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 300
  []
  [solid_heat_flux]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0
  []
[]

[Problem]
  type = BuoyantFoamProblem
  foam_temp = fluid_wall_temp
  heat_flux = solid_heat_flux  # sets fixed gradient BC on OpenFOAM solve
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 10
  dt = 0.5

  [TimeSteppers]
    [foam]
      type = FoamTimeStepper
    []
  []
[]

[Outputs]
  exodus = true
[]
