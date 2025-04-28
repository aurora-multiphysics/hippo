# This tests the ability of the HeatFluxAux to handle variable thermal conductivity.
# This case does not solve for a temperature field but initialises it using a
# pre-defined function for the temperature field. A linear thermal conductivity is
# used such it is 71 W/(m.K) at the left boundary and 142 W/(m.K) at the right
# boundary. The input temperature field results in a gradient of 100 K/m, hence the
# heat fluxes should be 7100 W/m^2 and -14200 W/m^2 normal to the left and right
# boundaries respectively.

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
        family = MONOMIAL
        order = FIRST
    []
[]

[Kernels]
    [T]
        type = NullKernel
        variable = T
    []
[]

[AuxVariables]
    [heat_flux]
        family = MONOMIAL
        order = FIRST
    []
[]

[ICs]
    [T]
        type = FunctionIC
        variable = T
        function = '300 + 100*x'
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

[Functions]
    [k]
        type = ParsedFunction
        expression = '71 + 71*x'
    []
[]
[Materials]
    [conduction_mat]
        type = GenericFunctionMaterial
        prop_names = 'thermal_conductivity'
        prop_values = 'k'
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
    []
    [right_boundary_hf]
        type = LineValueSampler
        num_points = 6
        start_point = '1 0 0'
        end_point = '1 1 0'
        sort_by = x
        variable = heat_flux
    []
[]

[Outputs]
    [csv]
        type = CSV
        execute_on = 'FINAL'
    []
    print_linear_residuals = false
[]
