[Mesh]
    [solid]
        type = GeneratedMeshGenerator
        dim = 3
        nx = 20
        ny = 5
        nz = 1
        xmin = 0.5
        xmax = 1.5
        ymin = 0
        ymax = 0.25
        zmin = 0
        zmax = 0.5
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
    []
[]

[Transfers]
    [T_from_fluid]
        type = MultiAppGeometricInterpolationTransfer
        source_variable = T
        from_multi_app = hippo
        variable = fluid_T
    []

    [T_to_fluid]
        # How do we do this transfer?
        # I guess we need a variable that holds the boundary values that we'll
        # copy to the MOOSE mesh?

        # This case is 'nice' in that the whole 'solid_top' boundary is
        # incident on the fluid domain. What happens if part of the boundary is
        # not incident on the fluid domain?
        # Maybe the 'transfer' will deal with this for us?

        # Also, how do we deal with having multiple solid/fluid domains?
        # I guess we can just define a transfer for every solid/fluid boundary.

        # Enabling this transfer (or the nearest node one) are causing a
        # libMesh access error... joy
        type = MultiAppGeometricInterpolationTransfer
        source_variable = temp
        to_multi_app = hippo
        variable = T
    []
[]

[Variables]
    [temp]
        family = LAGRANGE
        order = FIRST
        initial_condition = 500
    []
[]

[AuxVariables]
    [fluid_T]
        family = LAGRANGE
        order = FIRST
        initial_condition = 300
    []
[]

[Kernels]
    [heat-conduction]
        type = ADHeatConduction
        variable = temp
    []
    [heat-conduction-dt]
        type = ADHeatConductionTimeDerivative
        variable = temp
    []
[]

[BCs]
    [fluid_interface]
        type = CoupledVarNeumannBC
        variable = temp
        boundary = solid_top
        v = fluid_T
    []
[]

[Materials]
    [thermal-density]
        type = ADGenericConstantMaterial
        prop_names = 'density'
        prop_values = 0.2381
    []
    [thermal-conduction]
        type = ADHeatConductionMaterial
        specific_heat = 420
        thermal_conductivity = 50
    []
[]

[Executioner]
    type = Transient
    start_time = 0
    end_time = 1
    dt = 0.01

    solve_type = 'PJFNK'

    # petsc_options = '-snes_ksp_ew'
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'
    l_tol = 1e-6
    nl_abs_tol = 1e-9
    nl_rel_tol = 1e-8
[]

[Outputs]
    exodus = true
[]
