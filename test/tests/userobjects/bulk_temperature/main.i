[Mesh]
    type = GeneratedMesh
    xmin = -10
    xmax = 0
    nx=50
    ny=2
    nz=2
    dim=3
[]

[Variables]
    [T]
    []
[]

[Kernels]
    [dummy]
        type = NullKernel
        variable = T
    []
[]

[AuxVariables]
    [T_bulk]
    []
[]

[Executioner]
    type = Transient
    dt = 0.01
    end_time = 0.1
[]

[MultiApps]
    [hippo]
        type = TransientMultiApp
        input_files = adjacent_bulk_temperature.i
        execute_on = 'initial timestep_begin'
    []
[]

[Transfers]
    [T_bulk]
        type = MultiAppGeneralFieldUserObjectTransfer
        from_multi_app = hippo
        source_user_object = T_bulk
        variable = T_bulk
        to_boundaries = right
    []
[]

[Postprocessors]
  [adjacent_t_bulk]
    type = SideAverageValue
    variable = T_bulk
    boundary = right
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = True
[]
