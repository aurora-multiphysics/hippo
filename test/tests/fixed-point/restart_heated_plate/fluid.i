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
    [TimeSteppers]
        [foam]
            type = FoamControlledTimeStepper
        []
    []
[]

[Outputs]
    exodus = true
[]
