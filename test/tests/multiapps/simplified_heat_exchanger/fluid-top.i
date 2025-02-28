[Mesh]
    type = FoamMesh
    case = 'fluid-top-openfoam'
    foam_patch = 'interface'
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
        initial_condition = 3
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
    end_time = 1
    dt = 0.01

    [TimeSteppers]
        [foam]
            type = FoamTimeStepper
        []
    []
[]
