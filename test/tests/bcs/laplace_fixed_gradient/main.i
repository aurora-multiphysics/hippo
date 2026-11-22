# Both transfer paths prescribe the right gradient t, giving T = t*x.
# Source mesh encloses the existing OpenFOAM box, 0 <= x <= 10.
[Mesh]
    type = GeneratedMesh
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    xmax = 10
[]

[Problem]
    solve = false
[]

[Executioner]
    type = Transient
    dt = 1
    end_time = 32
[]

[Functions]
    [T_value]
        type = ParsedFunction
        expression = '0'
    []
    [T_flux]
        type = ParsedFunction
        expression = 't'
    []
[]

[AuxVariables]
    [T_value]
        family = LAGRANGE
        order = FIRST
    []
    [T_flux]
        family = LAGRANGE
        order = FIRST
    []
[]

# INITIAL MultiApp transfers run before INITIAL AuxKernels, so initialize sources with ICs.
[ICs]
    [T_value]
        type = FunctionIC
        variable = T_value
        function = T_value
    []
    [T_flux]
        type = FunctionIC
        variable = T_flux
        function = T_flux
    []
[]

[AuxKernels]
    [T_value]
        type = FunctionAux
        variable = T_value
        function = T_value
        execute_on = timestep_end
    []
    [T_flux]
        type = FunctionAux
        variable = T_flux
        function = T_flux
        execute_on = timestep_end
    []
[]

[MultiApps]
    [receiver]
        type = TransientMultiApp
        app_type = hippoApp
        input_files = child.i
        execute_on = 'initial timestep_end'
    []
[]

# Both modes sample the same source fields at the receiving face centres.
# Tests select the AuxVariable transfers and direct=false together through cli_args.
[Transfers]
    active = 'direct_T_value direct_T_flux'
    [direct_T_value]
        type = MultiApplibMeshToFoamTransfer
        to_multi_app = receiver
        libmesh_variable = T_value
        foam_variable = FoamBCs/T_value
        execute_on = 'initial timestep_end'
    []
    [aux_T_value]
        type = MultiAppGeneralFieldShapeEvaluationTransfer
        to_multi_app = receiver
        source_variable = T_value
        variable = T_value
        execute_on = 'initial timestep_end'
    []
    [direct_T_flux]
        type = MultiApplibMeshToFoamTransfer
        to_multi_app = receiver
        libmesh_variable = T_flux
        foam_variable = FoamBCs/T_flux
        execute_on = 'initial timestep_end'
    []
    [aux_T_flux]
        type = MultiAppGeneralFieldShapeEvaluationTransfer
        to_multi_app = receiver
        source_variable = T_flux
        variable = T_flux
        execute_on = 'initial timestep_end'
    []
[]
