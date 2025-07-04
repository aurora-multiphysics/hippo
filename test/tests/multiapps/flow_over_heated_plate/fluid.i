[Mesh]
    type = FoamMesh
    case = 'fluid-openfoam'
    foam_patch = 'interface'
[]

[Variables]
    [dummy]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 999
    []
[]

[AuxVariables]
    [fluid_wall_temp]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 300
    []
    [solid_heat_flux]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0
    []
[]

[FoamBCs]
    [foam_heat_flux_bc]
        # Take the heat flux from MOOSE and set it on the OpenFOAM mesh.
        moose_variable = solid_heat_flux
        foam_bc_type = fixedGradient
    []
[]

[FoamTransfer]
    [wall_temp]
        type = FromFoamBoundaryTransfer
        target_variable = fluid_wall_temp
        # Take the boundary temperature from OpenFOAM for setting on the MOOSE mesh.
        foam_variable = T
    []
[]

[Problem]
    type = FoamProblem
[]

[Executioner]
    type = Transient
    start_time = 0
    end_time = 1
    dt = 0.025

    solve_type = 'PJFNK'
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'

    [TimeSteppers]
        [foam]
            type = FoamTimeStepper
        []
    []
[]

[Outputs]
    exodus = true
[]
