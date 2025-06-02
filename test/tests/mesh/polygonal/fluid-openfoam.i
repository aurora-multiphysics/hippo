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
    [fluid_wall_temp]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0.075
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
