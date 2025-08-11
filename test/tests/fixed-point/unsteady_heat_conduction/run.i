[Mesh]
    [solid]
        type = GeneratedMeshGenerator
        dim = 3

        nx = 50
        xmin = -1.0
        xmax = 0.0

        ny = 1
        ymin = 0.0
        ymax = 0.1

        nz = 1
        zmin = 0.0
        zmax = 0.1

        # Calculated here:
        # https://openfoamwiki.net/index.php/Scripts/blockMesh_grading_calculation
        # Based on a grading factor of 25 and 100 elements.
        bias_x = 0.9364198516989446
    []
[]

[MultiApps]
    [hippo]
        type = TransientMultiApp
        app_type = hippoApp
        execute_on = TIMESTEP_BEGIN
        input_files = 'fluid-openfoam.i'
        sub_cycling = true
    []
[]

[Transfers]
    [wall_temperature_from_fluid]
        type = MultiAppGeneralFieldNearestLocationTransfer
        source_variable = fluid_wall_temp
        from_multi_app = hippo
        variable = fluid_wall_temperature
        execute_on = TIMESTEP_BEGIN
    []

    [heat_flux_to_fluid]
        type = MultiAppGeneralFieldNearestLocationTransfer
        source_variable = wall_heat_flux
        to_multi_app = hippo
        variable = solid_heat_flux
        execute_on = TIMESTEP_BEGIN
    []
[]

[Variables]
    [T]
        family = LAGRANGE
        order = FIRST
        initial_condition = 1
    []
[]

[AuxVariables]
    [fluid_wall_temperature]
        family = LAGRANGE
        order = FIRST
        initial_condition = 1
    []
    [wall_heat_flux]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0
    []
[]

[Kernels]
    [heat-conduction]
        type = HeatConduction
        variable = T
    []
    [heat-conduction-dt]
        type = HeatConductionTimeDerivative
        variable = T
    []
[]

[AuxKernels]
    [heat_flux_aux]
        type = HeatFluxAux
        variable = wall_heat_flux
        T = T
        boundary = right
        execute_on = 'INITIAL MULTIAPP_FIXED_POINT_BEGIN'
    []
[]

[BCs]
    [fluid_interface]
        type = MatchedValueBC
        variable = T
        boundary = right
        v = fluid_wall_temperature
    []
[]

[Materials]
    [thermal-conduction]
        type = HeatConductionMaterial
        thermal_conductivity = 1.0  # W/(m.K)
        specific_heat = 0.05  # J/(kg.K)
    []
    [thermal-density]
        type = GenericConstantMaterial
        prop_names  = 'density'
        prop_values = 20.0  # kg/m3
    []
  []

[Executioner]
    type = Transient
    start_time = 0
    end_time = 0.01
    dt = 0.0003125
    fixed_point_max_its=20
    fixed_point_min_its=1
    fixed_point_rel_tol=1e-3
    fixed_point_abs_tol=1e-8
    solve_type = 'PJFNK'
    petsc_options = '-snes_ksp_ew'
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'
    nl_abs_tol = 1e-7
    nl_rel_tol = 1e-8
[]

[Postprocessors]
    [temperature_integral]
        type = ElementIntegralVariablePostprocessor
        variable = T
    []
    [Tavg]
        type = SideAverageValue
        variable = T
        boundary=right
        execute_on = 'TIMESTEP_END'
    []
    [qw_avg]
        type = SideAverageValue
        variable = wall_heat_flux
        boundary=right
        execute_on = 'TIMESTEP_END'
    []

[]


[Outputs]
    exodus = true
[]
