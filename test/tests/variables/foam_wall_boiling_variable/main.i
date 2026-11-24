[Mesh]
  type = FoamMesh
  case = foam
  foam_patch = wall
[]
[Variables]
  [dummy]
    family = MONOMIAL
    order = CONSTANT
  []
[]
[Problem]
  type = FoamProblem
[]
[Executioner]
  type = Transient
  end_time = 0.001
  [TimeSteppers]
    [foam]
      type = FoamTimeStepper
    []
  []
[]
[FoamVariables]
  [flux]
    type = FoamWallHeatFlux
    wall_quantity = MULTIPHASE_EULER
    boundary = wall
  []
[]
[Outputs]
  exodus = true
[]
