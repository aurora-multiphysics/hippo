[Mesh]
    type = FileMesh
    file = 'solid.exo'
[]

[Variables]
    [T]
        family = LAGRANGE
        order = FIRST
        initial_condition = 300
    []
[]

[AuxVariables]
    [inner_heat_flux]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0
    []
    [outer_heat_flux]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0
    []
[]

[Kernels]
    [heat-conduction]
        type = ADHeatConduction
        variable = T
    []
    [heat-conduction-dt]
        type = ADHeatConductionTimeDerivative
        variable = T
    []
[]

[BCs]
    [inner]
        type = CoupledVarNeumannBC
        variable = T
        boundary = inner
        v = inner_heat_flux
        scale_factor=-1
    []

    [outer]
        type = CoupledVarNeumannBC
        variable = T
        boundary = outer
        v = outer_heat_flux
        scale_factor=-1
    []
[]

[Functions]
    [cp_func]
        type=ParsedFunction
        expression = 'if(t<50, 0.385, 385)'
    []
[]
[Materials]
    # The example specifies that the thermal diffusivity of the solid should
    # be α = 1 m2/s, and the thermal conductivity should be k = 100 W/(m.K).
    #
    # We know α = k/(Cp.ρ), where k is thermal conductivity, Cp is specific
    # heat capacity, and ρ is density.
    #
    # Hence we require that Cp.ρ = k = 100.
    [thermal-conduction]
        type = ADGenericConstantMaterial
        prop_names = 'thermal_conductivity density'
        prop_values = '401 8960'  # W/(m.K) kg/m^3
    []
    [specific-heat]
        type=ADGenericFunctionMaterial
        prop_names = 'specific_heat'
        prop_values = cp_func
    []
  []

[Executioner]
    type = Transient
    start_time = 0
    end_time = 500
    dt = 2.

    solve_type = 'NEWTON'
    petsc_options = '-snes_ksp_ew'
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'
    nl_rel_tol = 1e-7
    l_tol = 1e-8
    nl_abs_tol = 1e-7
[]

[Outputs]
    exodus = true
[]

[MultiApps]
    [inner]
        type = TransientMultiApp
        app_type = hippoApp
        execute_on = timestep_end
        input_files = 'inner.i'
        sub_cycling = true
    []
    [outer]
        type = TransientMultiApp
        app_type = hippoApp
        execute_on = timestep_end
        input_files = 'outer.i'
        sub_cycling = true
    []
[]

[Transfers]
    [wall_temperature_to_inner]
        type = MultiAppGeometricInterpolationTransfer
        source_variable = T
        to_multi_app = inner
        variable = solid_wall_temp
        execute_on = same_as_multiapp
    []

    [heat_flux_from_inner]
        type = MultiAppGeometricInterpolationTransfer
        source_variable = fluid_heat_flux
        from_multi_app = inner
        variable = inner_heat_flux
        execute_on = same_as_multiapp
    []

    [wall_temperature_to_outer]
        type = MultiAppGeometricInterpolationTransfer
        source_variable = T
        to_multi_app = outer
        variable = solid_wall_temp
        execute_on = same_as_multiapp
    []

    [heat_flux_from_outer]
        type = MultiAppGeometricInterpolationTransfer
        source_variable = fluid_heat_flux
        from_multi_app = outer
        variable = outer_heat_flux
        execute_on = same_as_multiapp
    []
[]
