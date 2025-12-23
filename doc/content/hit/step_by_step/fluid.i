[Mesh]
    type = FoamMesh
    case = 'fluid-openfoam'  # the directory of the OpenFOAM case
    foam_patch = 'interface' # the name of the coupled boundary
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
        type = FoamFixedGradientBC
        foam_variable = 'T'
        diffusivity_coefficient = kappa
        initial_condition = 0
    []
[]

[Problem]
    type = FoamProblem
[]

[Executioner]
    type = Transient
    start_time = 0
    end_time = 10
    dt = 0.025

    [TimeStepper]
        type = FoamTimeStepper
    []
[]
