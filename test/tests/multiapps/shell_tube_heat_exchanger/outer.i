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
    [fluid_heat_flux]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0
    []
[]

[Problem]
    type = FoamProblem
    # Take the heat flux from MOOSE and set it on the OpenFOAM mesh.
    foam_heat_flux = fluid_heat_flux
    # Take the boundary temperature from OpenFOAM and set it on the MOOSE mesh.
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
