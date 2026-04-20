[Mesh]
    type = FoamMesh
    case = 'foam'
    foam_patch = 'left'
[]

[FoamVariables]
    [wall_temp_monomial]
        type = FoamVariableField
        foam_variable = 'T'
        initial_condition = 0.075
    []
[]

[FoamBCs]
    [solid_heat_flux]
        type = FoamFixedGradientBC
        foam_variable = 'T'
        diffusivity = kappa
        initial_condition = 0
    []
[]

[Problem]
    type = FoamProblem
[]

[Executioner]
    type = Transient
    [TimeSteppers]
        [foam]
            type = FoamTimeStepper
        []
    []
[]

[Outputs]
    exodus = true
[]
