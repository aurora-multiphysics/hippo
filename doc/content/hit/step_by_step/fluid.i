[Mesh]
    type = FoamMesh
    case = 'fluid-openfoam'  # the directory of the OpenFOAM case
    foam_patch = 'interface' # the name of the coupled boundary
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

[Problem]
    type = FoamProblem
    # Take the heat flux from MOOSE and set it on the OpenFOAM mesh.
    heat_flux = solid_heat_flux
    # Take the boundary temperature from OpenFOAM and set it on the MOOSE mesh.
    foam_temp = fluid_wall_temp
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
