[Mesh]
  type = FoamMesh
  case = foam
  foam_patch = 'left right top bottom'
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
  end_time = 0.32
  [TimeSteppers/foam]
    type = FoamTimeStepper
  []
[]
[UserObjects]
  [cold_left]
    type = FixedBulkTemperature
    T_bulk = -1
  []
  [cold_right]
    type = FixedBulkTemperature
    T_bulk = -2
  []
[]
[Postprocessors]
  [temperature_right]
    type = FoamWallAverageTemperature
    boundary = 'right'
    execute_on = TIMESTEP_END
  []
  [temperature_left]
    type = FoamWallAverageTemperature
    boundary = 'left'
    execute_on = TIMESTEP_END
  []
  [temperature_top]
    type = FoamWallAverageTemperature
    boundary = 'top'
    execute_on = TIMESTEP_END
  []
  [temperature_combined]
    type = FoamWallAverageTemperature
    boundary = 'right top'
    execute_on = TIMESTEP_END
  []
  [temperature_opposite]
    type = FoamWallAverageTemperature
    boundary = 'right left'
    execute_on = TIMESTEP_END
  []
  [flux_right]
    type = FoamWallAverageHeatFlux
    boundary = 'right'
    execute_on = TIMESTEP_END
  []
  [flux_left]
    type = FoamWallAverageHeatFlux
    boundary = 'left'
    execute_on = TIMESTEP_END
  []
  [flux_top]
    type = FoamWallAverageHeatFlux
    boundary = 'top'
    execute_on = TIMESTEP_END
  []
  [flux_combined]
    type = FoamWallAverageHeatFlux
    boundary = 'right top'
    execute_on = TIMESTEP_END
  []
  [flux_opposite]
    type = FoamWallAverageHeatFlux
    boundary = 'right left'
    execute_on = TIMESTEP_END
  []
  [integral_right]
    type = FoamWallHeatFluxIntegral
    boundary = 'right'
    execute_on = TIMESTEP_END
  []
  [integral_left]
    type = FoamWallHeatFluxIntegral
    boundary = 'left'
    execute_on = TIMESTEP_END
  []
  [integral_top]
    type = FoamWallHeatFluxIntegral
    boundary = 'top'
    execute_on = TIMESTEP_END
  []
  [integral_combined]
    type = FoamWallHeatFluxIntegral
    boundary = 'right top'
    execute_on = TIMESTEP_END
  []
  [integral_opposite]
    type = FoamWallHeatFluxIntegral
    boundary = 'right left'
    execute_on = TIMESTEP_END
  []
  [htc_right]
    type = FoamWallAverageHTC
    boundary = 'right'
    execute_on = TIMESTEP_END
    bulk_temperature_uo = 'cold_right'
  []
  [htc_left]
    type = FoamWallAverageHTC
    boundary = 'left'
    execute_on = TIMESTEP_END
    bulk_temperature_uo = 'cold_left'
  []
  [htc_opposite]
    type = FoamWallAverageHTC
    boundary = 'right left'
    execute_on = TIMESTEP_END
    bulk_temperature_uo = 'cold_right cold_left'
  []
  [htc_combined]
    type = FoamWallAverageHTC
    boundary = 'right top'
    execute_on = TIMESTEP_END
    bulk_temperature_uo = 'cold_right cold_left'
  []
[]
[Outputs]
  csv = true
[]
