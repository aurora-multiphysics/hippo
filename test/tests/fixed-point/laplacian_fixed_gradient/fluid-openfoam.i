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
    [fluid_heat_flux]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0.075
    []
[]

[AuxKernels]
    [fluid_heat_flux]
        type = ParsedAux
        variable = fluid_heat_flux
        expression = '1000*t'
        use_xyzt = true
    []
[]

[Problem]
    type = FoamProblem
    # Take the boundary temperature from OpenFOAM and set it on the MOOSE mesh.
    heat_flux = fluid_heat_flux
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
    exodus = true
[]
