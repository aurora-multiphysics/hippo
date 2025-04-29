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
