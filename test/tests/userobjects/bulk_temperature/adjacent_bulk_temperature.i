
[Mesh]
  type = FoamMesh
  case = adjacent_cell
  foam_patch = left
[]

[Variables]
  [Dummy]
    initial_condition = 3.14
  []
[]

[Problem]
    type = FoamProblem
[]

[AuxVariables]
  [T_bulk]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[UserObjects]
  [T_bulk]
    type = AdjacentCellBulkTemperature
    boundary = left
    execute_on = 'initial timestep_end'
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
  type = Transient
  dt = 0.01
  end_time = 0.1
  [TimeStepper]
    type = FoamTimeStepper
  []
[]

[Postprocessors]
  [adjacent_t_bulk]
    type = ElementAverageValue
    variable = T_bulk
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = True
[]
