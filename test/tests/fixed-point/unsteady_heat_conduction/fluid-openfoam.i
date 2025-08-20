[Mesh]
    type = FoamMesh
    case = 'foam'
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
        family = LAGRANGE
        order = FIRST
        initial_condition = 0.075
    []
    [wall_temp_monomial]
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

[AuxKernels]
    [fluid_wall_temp]
        type= ProjectionAux
        variable = fluid_wall_temp
        v = wall_temp_monomial
    []
[]
[Problem]
    type = FoamProblem
    # Take the heat flux from MOOSE and set it on the OpenFOAM mesh.
    heat_flux = solid_heat_flux
    # Take the boundary temperature from OpenFOAM and set it on the MOOSE mesh.
    foam_temp = wall_temp_monomial
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
