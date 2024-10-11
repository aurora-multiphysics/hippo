[Mesh]
    [solid]
        type = GeneratedMeshGenerator
        dim = 3
        nx = 50
        ny = 50
        nz = 1
        xmin = 0
        xmax = 2
        ymin = 0
        ymax = 1
        zmin = 0
        zmax = 1
        elem_type = HEX8
        boundary_name_prefix = solid
    []
[]

[MultiApps]
    [hippo]
        type = TransientMultiApp
        app_type = hippoApp
        execute_on = timestep_begin
        input_files = 'fluid.i'
        # sub_cycling = true
    []
[]

[Transfers]
    [T_from_fluid]
        type = MultiAppGeometricInterpolationTransfer
        source_variable = fluid_wall_temp
        from_multi_app = hippo
        variable = fluid_T
    []

    [hf_to_fluid]
        type = MultiAppGeometricInterpolationTransfer
        source_variable = heat_flux
        to_multi_app = hippo
        variable = solid_heat_flux
    []
[]

[Variables]
    [temp]
        family = LAGRANGE
        order = FIRST
        initial_condition = 310
    []
[]

[AuxVariables]
    [fluid_T]
        family = LAGRANGE
        order = FIRST
        initial_condition = 300
    []
    [heat_flux]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 100
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
    [heat_flux_kernel]
        type = HeatFluxAux
        variable = heat_flux
        T = temp
        thermal_conductivity = 100
        boundary = 'solid_top'
    []
[]

[BCs]
    [fluid_interface]
        type = MatchedValueBC
        variable = temp
        boundary = solid_top
        v = fluid_T
    []
[]

[Materials]
    [thermal-conduction]
        type = HeatConductionMaterial
        thermal_conductivity = 100
        specific_heat = 200
    []
    [thermal-density]
        type = GenericConstantMaterial
        prop_names = 'density'
        prop_values = 200
    []
[]

[Executioner]
    type = Transient
    start_time = 0
    end_time = 10
    dt = 0.5

    solve_type = 'PJFNK'
    petsc_options = '-snes_ksp_ew'
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'
    nl_abs_tol = 1e-9
    nl_rel_tol = 1e-8
[]

# [VectorPostprocessors]
#     [interface_values]
#         type = LineValueSampler
#         num_points = 20
#         start_point = '0 1 0.5'
#         end_point =   '2 1 0.5'
#         sort_by = 'x'
#         variable = 'temp heat_flux fluid_T'
#         execute_on = timestep_end
#     []
#     [midline_temp]
#         type = LineValueSampler
#         num_points = 20
#         start_point = '1 0 0.5'
#         end_point =   '1 1 0.5'
#         sort_by = 'x'
#         variable = 'temp'
#         execute_on = timestep_end
#     []
# []

[Outputs]
    exodus = true
    # csv = true
[]
