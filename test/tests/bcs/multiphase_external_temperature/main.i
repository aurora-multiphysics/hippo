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

[FoamBCs]
  [heat_flux]
    type = FoamMultiphaseExternalTemperatureBC
    boundary = wall
  []
[]

[AuxKernels]
  [heat_flux]
    type = ParsedAux
    variable = heat_flux
    expression = '73890'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]
[Problem]
  type = FoamProblem
[]
[Executioner]
  type = Transient
  end_time = 0.02
  [TimeSteppers]
    [foam]
      type = FoamTimeStepper
    []
  []
[]
