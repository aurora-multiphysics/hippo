
[Mesh]
  type = GeneratedMesh
  dim=3
[]

[Variables]
  [Dummy]
    initial_condition = 3.14
  []
[]

[AuxVariables]
  [T_bulk]
  []
[]

[UserObjects]
  [T_bulk]
    type = FixedBulkTemperature
    T_bulk = 298.15
  []
[]

[AuxKernels]
  [T_bulk]
    type = SpatialUserObjectAux
    user_object=T_bulk
    variable = T_bulk
    execute_on = 'initial timestep_end'
  []
[]

[Kernels]
  [null]
    type=NullKernel
    variable = Dummy
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [t_bulk]
    type = ElementAverageValue
    variable = T_bulk
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv=true
[]
