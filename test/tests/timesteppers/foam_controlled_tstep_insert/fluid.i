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

[AuxVariables]
  [T]
    initial_condition = 111
    family = MONOMIAL
    order = CONSTANT
  []
[]

[FoamBCs]
  [T]
    type = FoamFixedValueBC
    foam_variable = T
    v = T
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

  end_time=3.
  dt = 1.
  [TimeStepper]
    type = FoamControlledTimeStepper
  []
[]

[Outputs]
  exodus = true
[]
