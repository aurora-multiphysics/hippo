[Mesh]
    type = GeneratedMesh
    dim=3
[]

[Variables]
    [dummy]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 999
    []
[]

[Kernels]
    [dummy]
        type = NullKernel
        variable = dummy
    []
[]

[MultiApps]
    [hippo]
        type = TransientMultiApp
        app_type = hippoApp
        execute_on = timestep_begin
        input_files = 'fluid-openfoam.i'
        sub_cycling = false
    []
[]

[Executioner]
    type = Transient
    start_time = 0
    end_time = 0.01
    dt = 0.0003125
    fixed_point_min_its=2
    fixed_point_max_its=2
[]

[Outputs]
    exodus = true
[]
