[Mesh]
    [solid]
        type = GeneratedMeshGenerator
        boundary_name_prefix = solid
        dim = 3

        nx = 161
        xmin = 0
        xmax = 1

        ny = 16
        ymin = -0.25
        ymax = 0

        nz = 1
        zmin = 0
        zmax = 0.4

        # Calculated here:
        # https://openfoamwiki.net/index.php/Scripts/blockMesh_grading_calculation
        # Based on a grading factor of 5 and 161 elements.
        bias_x = 1.010109749
    []
[]

[Variables]
    [temp]
        family = LAGRANGE
        order = FIRST
        initial_condition = 310
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

[BCs]
    [fixed_temp]
        type = DirichletBC
        variable = temp
        boundary = solid_bottom
        value = 310
    []

    # Use the fluid wall temperature as a matched value boundary condition.
    [fluid_interface]
        type = MatchedValueBC
        variable = temp
        boundary = solid_top
        v = fluid_wall_temperature
    []
[]

[Materials]
    # The example specifies that the thermal diffusivity of the solid should
    # be α = 1 m2/s, and the thermal conductivity should be k = 100 W/(m.K).
    #
    # We know α = k/(ρ.Cp), where k is thermal conductivity, Cp is specific
    # heat capacity, and ρ is density.
    #
    # Hence we require that ρ.Cp = k = 100.
    [thermal-conduction]
        type = HeatConductionMaterial
        thermal_conductivity = 100.0  # W/(m.K)
        specific_heat = 0.5  # J/(kg.K)
    []
    [thermal-density]
        type = GenericConstantMaterial
        prop_names  = 'density'
        prop_values = 200.0  # kg/m3
    []
[]

[AuxVariables]
    [wall_heat_flux]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0
    []

    # Add an AuxVariable to store the wall temperature of the fluid domain.
    [fluid_wall_temperature]
        family = LAGRANGE
        order = FIRST
        initial_condition = 0
    []
[]

[AuxKernels]
    [heat_flux_aux]
        type = HeatFluxAux
        variable = wall_heat_flux
        thermal_conductivity = 100  # W/(m.K)
        T = temp
        boundary = 'solid_top'
    []
[]

[MultiApps]
    [hippo]
        type = TransientMultiApp
        app_type = hippoApp
        execute_on = timestep_begin
        input_files = 'fluid.i'
    []
[]

[Transfers]
    # Copy the wall temperature from the fluid into an AuxVariable.
    [wall_temperature_from_fluid]
        type = MultiAppGeometricInterpolationTransfer
        source_variable = fluid_wall_temp
        from_multi_app = hippo
        variable = fluid_wall_temperature
        execute_on = same_as_multiapp
    []

    # Copy the heat flux from the 'wall_heat_flux' aux variable into the
    # multiapp.
    # Remember we marked the 'solid_heat_flux' variable in 'fluid.i' to be
    # used as a heat flux boundary condition on the OpenFOAM solve.
    [heat_flux_to_fluid]
        type = MultiAppGeometricInterpolationTransfer
        source_variable = wall_heat_flux
        to_multi_app = hippo
        variable = solid_heat_flux
        execute_on = same_as_multiapp
    []
[]

[Executioner]
    type = Transient
    start_time = 0
    end_time = 10
    dt = 0.025

    fixed_point_abs_tol = 1e-7
    fixed_point_rel_tol = 1e-8

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
