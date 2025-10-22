[Mesh]
  type = FoamMesh
  case = 'fluid-openfoam'
  foam_patch = 'interface'
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

  end_time=0.5
  [TimeStepper]
    type = FoamControlledTimeStepper
  []
[]

[Outputs]
  exodus = true
[]
