[Mesh]
  type = FoamMesh
  case = foam
  foam_patch = 'left right top front bottom back'
[]
[Variables]
  [dummy]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 999
  []
[]
[Problem]
  type = FoamProblem
[]
[Executioner]
  type = Transient
  end_time = 0.03
  [TimeSteppers]
    [foam]
      type = FoamTimeStepper
    []
  []
[]
[FoamVariables]
  [temperature]
    type = FoamWallTemperature
    wall_quantity = MULTIPHASE_EULER
    boundary = 'left right top front bottom back'
  []
  [flux]
    type = FoamWallHeatFlux
    wall_quantity = MULTIPHASE_EULER
    boundary = 'left right top front bottom back'
  []
  [htc]
    type = FoamHeatTransferCoeff
    wall_quantity = MULTIPHASE_EULER
    boundary = 'left right top front bottom back'
    bulk_temperature_uo = 'fixed fixed fixed fixed fixed fixed'
  []
[]
[UserObjects]
  [fixed]
    type = FixedBulkTemperature
    T_bulk = 290
  []
  [bulk_left]
    type = AdjacentCellBulkTemperature
    boundary = left
    wall_quantity = MULTIPHASE_EULER
    execute_on = 'initial timestep_end'
  []
  [bulk_right]
    type = AdjacentCellBulkTemperature
    boundary = right
    wall_quantity = MULTIPHASE_EULER
    execute_on = 'initial timestep_end'
  []
  [bulk_bottom]
    type = AdjacentCellBulkTemperature
    boundary = bottom
    wall_quantity = MULTIPHASE_EULER
    execute_on = 'initial timestep_end'
  []
  [bulk_top]
    type = AdjacentCellBulkTemperature
    boundary = top
    wall_quantity = MULTIPHASE_EULER
    execute_on = 'initial timestep_end'
  []
  [bulk_back]
    type = AdjacentCellBulkTemperature
    boundary = back
    wall_quantity = MULTIPHASE_EULER
    execute_on = 'initial timestep_end'
  []
  [bulk_front]
    type = AdjacentCellBulkTemperature
    boundary = front
    wall_quantity = MULTIPHASE_EULER
    execute_on = 'initial timestep_end'
  []
[]
[AuxVariables]
  [bulk]
    family = MONOMIAL
    order = CONSTANT
  []
[]
[AuxKernels]
  [bulk_left]
    type = SpatialUserObjectAux
    user_object = bulk_left
    variable = bulk
    block = left
    execute_on = 'initial timestep_end'
  []
  [bulk_right]
    type = SpatialUserObjectAux
    user_object = bulk_right
    variable = bulk
    block = right
    execute_on = 'initial timestep_end'
  []
  [bulk_bottom]
    type = SpatialUserObjectAux
    user_object = bulk_bottom
    variable = bulk
    block = bottom
    execute_on = 'initial timestep_end'
  []
  [bulk_top]
    type = SpatialUserObjectAux
    user_object = bulk_top
    variable = bulk
    block = top
    execute_on = 'initial timestep_end'
  []
  [bulk_back]
    type = SpatialUserObjectAux
    user_object = bulk_back
    variable = bulk
    block = back
    execute_on = 'initial timestep_end'
  []
  [bulk_front]
    type = SpatialUserObjectAux
    user_object = bulk_front
    variable = bulk
    block = front
    execute_on = 'initial timestep_end'
  []
[]
[Outputs]
  exodus = true
[]
