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

[FoamVariables]
    [wall_temp_monomial]
        type = FoamVariableField
        foam_variable = 'T'
        initial_condition = 0.075
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

[AuxVariables]
    [fluid_wall_temp]
        family = LAGRANGE
        order = FIRST
        initial_condition = 0.075
    []
[]

[AuxKernels]
    [fluid_wall_temp]
        type= ProjectionAux
        variable = fluid_wall_temp
        v = wall_temp_monomial
    []
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
