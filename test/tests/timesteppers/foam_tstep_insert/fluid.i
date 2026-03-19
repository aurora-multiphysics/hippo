[Mesh]
  type = FoamMesh
  case = 'buoyantCavity'
  foam_patch = 'topAndBottom frontAndBack'
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
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  dt=0.4
  end_time=3.
  [TimeStepper]
    type = FoamTimeStepper
  []
[]

[Outputs]
  exodus = true
[]
