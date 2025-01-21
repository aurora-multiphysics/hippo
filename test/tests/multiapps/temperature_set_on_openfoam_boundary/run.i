[Mesh]
    [solid_vertical]
        type = GeneratedMeshGenerator
        dim = 3
        nx = 5
        ny = 10
        nz = 5
        xmin = -0.01
        xmax = 0.0
        ymin = 0.01
        ymax = 0.02
        zmin = 0.0
        zmax = 0.01
        boundary_name_prefix = vertical
    []
    [solid_horizontal]
        type = GeneratedMeshGenerator
        dim = 3
        nx = 10
        ny = 5
        nz = 5
        xmin = 0
        xmax = 0.02
        ymin = 0.02
        ymax = 0.025
        zmin = 0
        zmax = 0.01
        boundary_name_prefix = horizontal
    []
    [solid_corner]
        type = GeneratedMeshGenerator
        dim = 3
        nx = 5
        ny = 5
        nz = 5
        xmin = -0.01
        xmax = 0.0
        ymin = 0.02
        ymax = 0.025
        zmin = 0
        zmax = 0.01
        boundary_name_prefix = corner
    []
    [solid]
        type = StitchedMeshGenerator
        inputs = 'solid_vertical solid_corner solid_horizontal'
        clear_stitched_boundary_ids = true
        stitch_boundaries_pairs = 'vertical_top corner_bottom;
                                   corner_right horizontal_left'
    []
[]

[Variables]
    [temp]
    []
[]

[Kernels]
    [null]
        type = NullKernel
        variable = temp
    []
[]

[ICs]
    [temp_ic]
        type = FunctionIC
        variable = temp
        function = sum_coords
    []
[]

[MultiApps]
    [hippo]
        type = TransientMultiApp
        app_type = hippoApp
        execute_on = timestep_begin
        input_files = fluid.i
    []
[]

[Transfers]
    [wall_temperature_to_fluid]
        type = MultiAppShapeEvaluationTransfer
        source_variable = temp
        to_multi_app = hippo
        variable = T
    []
[]

[Functions]
    [sum_coords]
        type = ParsedFunction
        expression = '300 + x + y + z'
    []
[]

[Executioner]
    type = Transient
    start_time = 0
    end_time = 0.1
    dt = 0.1

    solve_type = 'PJFNK'

    petsc_options = '-snes_ksp_ew'
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
    exodus = true
[]
