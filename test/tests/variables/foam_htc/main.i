[Mesh]
    type = FoamMesh
    case = 'foam'
    foam_patch = 'left'
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
    end_time = 0.32
    [TimeSteppers]
        [foam]
            type = FoamTimeStepper
        []
    []
[]

[FoamVariables]
  [htc]
    type = FoamHeatTransferCoeff
    bulk_temperature_uo = bulk_t_uo
    boundary = left
  []
[]

[UserObjects]
  [bulk_t_uo]
    type = FixedBulkTemperature
    T_bulk = 1
  []
[]

[Outputs]
    exodus = true
    csv = true
[]
