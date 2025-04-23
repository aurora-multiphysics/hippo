[Mesh]
    [solid]
        type = GeneratedMeshGenerator
        dim = 3

        nx = 161
        xmin = 0
        xmax = 1

        ny = 41
        ymin = -0.5
        ymax = 0

        nz = 1
        zmin = 0.0
        zmax = 0.4

        # Calculated here:
        # https://openfoamwiki.net/index.php/Scripts/blockMesh_grading_calculation
        # Based on a grading factor of 5 and 161 elements.
        bias_x = 1.010109749
    []
[]

[MultiApps]
    [fluid_bottom]
        type = TransientMultiApp
        app_type = hippoApp
        execute_on = timestep_begin
        input_files = 'fluid-bottom.i'
    []

    [fluid_top]
        type = TransientMultiApp
        app_type = hippoApp
        execute_on = timestep_begin
        input_files = 'fluid-top.i'
    []
[]

[Transfers]
    [top_wall_temperature_from_fluid]
        type = MultiAppGeneralFieldShapeEvaluationTransfer
        source_variable = fluid_wall_temp
        from_multi_app = fluid_top
        variable = top_fluid_wall_temperature
        execute_on = same_as_multiapp
    []

    [top_heat_flux_to_fluid]
        type = MultiAppGeneralFieldShapeEvaluationTransfer
        source_variable = top_wall_heat_flux
        to_multi_app = fluid_top
        variable = solid_heat_flux
        execute_on = same_as_multiapp
    []

    [bottom_wall_temperature_from_fluid]
        type = MultiAppGeneralFieldShapeEvaluationTransfer
        source_variable = fluid_wall_temp
        from_multi_app = fluid_bottom
        variable = bottom_fluid_wall_temperature
        execute_on = same_as_multiapp
    []

    [bottom_heat_flux_to_fluid]
        type = MultiAppGeneralFieldShapeEvaluationTransfer
        source_variable = bottom_wall_heat_flux
        to_multi_app = fluid_bottom
        variable = solid_heat_flux
        execute_on = same_as_multiapp
    []
[]

[Variables]
    [temp]
        family = LAGRANGE
        order = FIRST
        initial_condition = 305
    []
[]

[AuxVariables]
    [top_fluid_wall_temperature]
        family = LAGRANGE
        order = FIRST
        initial_condition = 300
    []

    [top_wall_heat_flux]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0
    []

    [bottom_fluid_wall_temperature]
        family = LAGRANGE
        order = FIRST
        initial_condition = 310
    []

    [bottom_wall_heat_flux]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0
    []
[]

[Kernels]
    [heat-conduction]
        type = HeatConduction
        variable = temp
    []
    [heat-conduction-dt]
        type = HeatConductionTimeDerivative
        variable = temp
    []
[]

[AuxKernels]
    [top_heat_flux_aux]
        type = HeatFluxAux
        variable = top_wall_heat_flux
        T = temp
        boundary = top
    []

    [bottom_heat_flux_aux]
        type = HeatFluxAux
        variable = bottom_wall_heat_flux
        T = temp
        boundary = bottom
    []
[]

[BCs]
    [top_interface]
        type = MatchedValueBC
        variable = temp
        boundary = top
        v = top_fluid_wall_temperature
    []

    [bottom_interface]
        type = MatchedValueBC
        variable = temp
        boundary = bottom
        v = bottom_fluid_wall_temperature
    []
[]

[Materials]
    [thermal-conduction]
        type = HeatConductionMaterial
        thermal_conductivity = 100.0  # W/(m.K)
        specific_heat = 100.0  # J/(kg.K)
    []

    [thermal-density]
        type = GenericConstantMaterial
        prop_names  = 'density'
        prop_values = 1.0  # kg/m3
    []
  []

[Executioner]
    type = Transient
    start_time = 0
    end_time = 1
    dt = 0.01

    solve_type = 'PJFNK'
    petsc_options = '-snes_ksp_ew'
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'
    nl_abs_tol = 1e-7
    nl_rel_tol = 1e-8
[]

[Outputs]
    exodus = true
[]
