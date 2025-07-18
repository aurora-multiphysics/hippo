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

[AuxVariables]
    [solid_wall_temp]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 300
    []
    [fluid_heat_flux]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0
    []
[]

[Problem]
    type = FoamProblem
    # Assign heat flux computed in OpenFOAM to a MOOSE variable for imposition as BC in MOOSE.
    foam_heat_flux = fluid_heat_flux
    # Take the boundary temperature from MOOSE and set it in OpenFOAM.
    temp = solid_wall_temp
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
