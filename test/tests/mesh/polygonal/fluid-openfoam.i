[Mesh]
    type = FoamMesh
    case = 'fluid-openfoam'
    foam_patch = 'left'
[]

[Variables]
    [dummy]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 999
    []
[]


[AuxVariables]
    [solid_heat_flux]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0
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
    end_time = 0.01
    dt = 0.0003125

    [TimeSteppers]
        [foam]
            type = FoamTimeStepper
        []
    []
[]

[Outputs]
    exodus = false
[]
