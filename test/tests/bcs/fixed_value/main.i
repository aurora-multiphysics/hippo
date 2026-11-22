# Compare both transfer paths with the same prescribed boundary temperatures.
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
    dt = 0.01
    end_time = 0.32
[]

# These functions can be represented exactly by the mesh.
[Functions]
    [temp1]
        type = ParsedFunction
        expression = '0.05 + (x+y+z)*t'
    []
    [temp2]
        type = ParsedFunction
        expression = '0.05 + 2*(x+y+z)*t'
    []
[]

[AuxVariables]
    [temp1]
        family = LAGRANGE
        order = FIRST
    []
    [temp2]
        family = LAGRANGE
        order = FIRST
    []
[]

# INITIAL MultiApp transfers run before INITIAL AuxKernels, so initialize sources with ICs.
[ICs]
    [temp1]
        type = FunctionIC
        variable = temp1
        function = temp1
    []
    [temp2]
        type = FunctionIC
        variable = temp2
        function = temp2
    []
[]

[AuxKernels]
    [temp1]
        type = FunctionAux
        variable = temp1
        function = temp1
        execute_on = timestep_end
    []
    [temp2]
        type = FunctionAux
        variable = temp2
        function = temp2
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
    active = 'direct_temp1 direct_temp2'
    [direct_temp1]
        type = MultiApplibMeshToFoamTransfer
        to_multi_app = receiver
        libmesh_variable = temp1
        foam_variable = FoamBCs/temp1
        execute_on = 'initial timestep_end'
    []
    [aux_temp1]
        type = MultiAppGeneralFieldShapeEvaluationTransfer
        to_multi_app = receiver
        source_variable = temp1
        variable = temp1
        execute_on = 'initial timestep_end'
    []
    [direct_temp2]
        type = MultiApplibMeshToFoamTransfer
        to_multi_app = receiver
        libmesh_variable = temp2
        foam_variable = FoamBCs/temp2
        execute_on = 'initial timestep_end'
    []
    [aux_temp2]
        type = MultiAppGeneralFieldShapeEvaluationTransfer
        to_multi_app = receiver
        source_variable = temp2
        variable = temp2
        execute_on = 'initial timestep_end'
    []
[]
