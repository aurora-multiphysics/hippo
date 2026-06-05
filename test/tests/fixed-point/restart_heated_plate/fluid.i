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

[FoamVariables]
    [fluid_wall_temp]
        type = FoamVariableField
        foam_variable = 'T'
        initial_condition = 300
    []
[]

[FoamBCs]
    [solid_heat_flux]
        type = FoamDiffusionFluxBC
        foam_variable = 'T'
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
