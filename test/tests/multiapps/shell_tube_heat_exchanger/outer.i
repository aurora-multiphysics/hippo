[Mesh]
    type = FoamMesh
    case = 'fluid_outer'
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
    [solid_wall_temp]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 300
    []
[]

[FoamBCs]
    [temp]
        type = FoamFixedValueBC
        foam_variable = T
        v = solid_wall_temp
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
