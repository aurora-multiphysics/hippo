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
        execute_on = timestep_begin
        input_files = 'fluid-openfoam.i'
        sub_cycling = true
    []
[]

[Transfers]
    [wall_temperature_from_fluid]
        type = MultiAppShapeEvaluationTransfer
        source_variable = fluid_wall_temp
        from_multi_app = hippo
        variable = fluid_wall_temperature
        execute_on = same_as_multiapp
    []

    [heat_flux_to_fluid]
        type = MultiAppShapeEvaluationTransfer
        source_variable = wall_heat_flux
        to_multi_app = hippo
        variable = solid_heat_flux
        execute_on = same_as_multiapp
    []
[]

[Variables]
    [temp]
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
        variable = temp
    []
    [heat-conduction-dt]
        type = HeatConductionTimeDerivative
        variable = temp
    []
[]

[AuxKernels]
    [heat_flux_aux]
        type = HeatFluxAux
        variable = wall_heat_flux
        T = temp
        boundary = right
    []
[]

[BCs]
    [fluid_interface]
        type = MatchedValueBC
        variable = temp
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
        variable = temp
    []
[]

[VectorPostprocessors]
    [temp_over_line]
        type = LineValueSampler
        variable = temp
        start_point = '-1.0 0.05 0'
        end_point = '0 0.05 0'
        num_points = 50
        sort_by = 'x'
    []
[]

[Outputs]
    exodus = true

    [temp_over_line_csv]
        type = CSV
        time_column = true
        time_data = true
    []
[]
