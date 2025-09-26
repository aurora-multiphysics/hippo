[Mesh]
    type = FoamMesh
    case = 'fluid-top-openfoam'
    foam_patch = 'interface'
[]

[AuxVariables]
    [solid_heat_flux]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 3
    []
[]

[FoamBCs]
    [heat_flux]
        type = FoamFixedGradientBC
        foam_variable = T
        v = solid_heat_flux
        diffusivity_coefficient = kappa
    []
[]

[FoamVariables]
    [fluid_wall_temp]
        type = FoamVariableField
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
    dt = 0.01

    [TimeSteppers]
        [foam]
            type = FoamTimeStepper
        []
    []
[]
