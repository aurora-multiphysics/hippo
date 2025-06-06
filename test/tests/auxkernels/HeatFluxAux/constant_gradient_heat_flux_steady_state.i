# In this example we have a 2D mesh with a temperature gradient. The left side
# is held at 300 K, the right side at 400 K, and the sides are 1 m apart.
# This results in a temperature gradient of 100 K/m, in the positive x
# direction.
# If we set the thermal conductivity k := 71 W/(m.K), by Fourier's law of
# conduction, we expect the heat flux to be q = -71*100 on the right boundary,
# and the 71*100 at the left boundary.

[Mesh]
    type = GeneratedMesh
    dim = 2
    nx = 10
    ny = 10
    xmax = 1.0
    ymax = 1.0
[]

[Variables]
    [T]
        initial_condition = 300
    []
[]

[Kernels]
    [heat_conduction]
        type = HeatConduction
        variable = T
    []
[]

[BCs]
    [left_bc]
        type = DirichletBC
        variable = T
        boundary = left
        value = 300
    []
    [right_bc]
        type = DirichletBC
        variable = T
        boundary = right
        value = 400
    []
[]

[AuxVariables]
    [heat_flux]
        family = MONOMIAL
        order = FIRST
    []
[]

[AuxKernels]
    [heat_flux_aux]
        type = HeatFluxAux
        variable = heat_flux
        T = T
        boundary = 'left right'
    []
[]

[Materials]
    [conduction_mat]
        type = HeatConductionMaterial
        thermal_conductivity = 71
    []
    [thermal_mat]
        type = GenericConstantMaterial
        prop_names  = density
        prop_values = 200.0  # kg/m3
    []
[]

[Executioner]
    type = Steady

    solve_type = 'PJFNK'
    petsc_options = '-snes_ksp_ew'
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'
[]

[VectorPostprocessors]
    [left_boundary_hf]
        type = LineValueSampler
        num_points = 6
        start_point = '0 0 0'
        end_point = '0 1 0'
        sort_by = x
        variable = heat_flux
        warn_discontinuous_face_values = false
    []
    [right_boundary_hf]
        type = LineValueSampler
        num_points = 6
        start_point = '1 0 0'
        end_point = '1 1 0'
        sort_by = x
        variable = heat_flux
        warn_discontinuous_face_values = false
    []
[]

[Outputs]
    [csv]
        type = CSV
        execute_on = 'FINAL'
    []
    print_linear_residuals = false
[]
