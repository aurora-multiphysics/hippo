[Mesh]
  type = FoamMesh
  case = 'buoyantCavity'
  foam_patch = 'patch2 patch4'
[]

[Variables]
  [Dummy]
    initial_condition = 3.14
  []
[]

[FoamBCs]
  [T]
    type = FoamFixedValueBC
    foam_variable = T
    initial_condition = 111
  []
[]

[FoamVariables]
  [foam_T]
    type = FoamVariableField
    foam_variable = T
  []
  [foam_hf]
    type = FoamFunctionObject
    foam_variable = wallHeatFlux
  []
[]

[Problem]
  type=FoamProblem
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 0.1
  dt = 0.1

  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  [TimeStepper]
    type = FoamTimeStepper
  []
[]

[Outputs]
  exodus = true
[]
