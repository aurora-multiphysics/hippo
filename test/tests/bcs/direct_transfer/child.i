[Mesh]
  type = FoamMesh
  case = child_case
  # Direct transfers also target patches outside this mirrored boundary.
  foam_patch = bottom
[]

[Problem]
  type = FoamProblem
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 4
[]

[FoamBCs]
  [prescribed_value]
    type = FoamFixedValueBC
    foam_variable = T
    boundary = 'left right bottom top'
  []
  [prescribed_gradient]
    type = FoamFixedGradientBC
    foam_variable = T
    boundary = front
  []
  [prescribed_flux]
    type = FoamDiffusionFluxBC
    foam_variable = T
    boundary = back
  []
[]
