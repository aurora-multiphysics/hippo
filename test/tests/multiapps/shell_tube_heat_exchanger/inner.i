[Mesh]
    type = FoamMesh
    case = 'fluid_inner'
    foam_patch = 'interface'
[]

[Variables]
    [dummy]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 999
    []
[]

[FoamBCs]
    [solid_wall_temp]
        type = FoamFixedValueBC
        foam_variable = T
        initial_condition = 300
    []
[]

[FoamVariables]
    [fluid_heat_flux]
        type = FoamFunctionObject
        foam_variable = wallHeatFlux
    []
[]


[Problem]
    type = FoamProblem
[]

[Executioner]
    type = Transient
    start_time = 0
    end_time = 500
    dt = 1.

    [TimeSteppers]
        [foam]
            type = FoamTimeStepper
        []
    []
[]

[Outputs]
    exodus = false
[]
