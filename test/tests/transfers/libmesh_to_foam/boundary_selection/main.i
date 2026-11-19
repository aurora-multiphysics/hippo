[Mesh]
  type = FoamMesh
  case = parent_case
  foam_patch = 'left right bottom top front back'
[]
[Problem]
  type = FoamProblem
  solve = true
[]
[Variables]
  [dummy]
    family = MONOMIAL
    order = CONSTANT
  []
[]
[Executioner]
  type = Transient
  dt = 1
  end_time = 3
[]
[Functions]
  [source_function]
    type = ParsedFunction
    expression = '10 + 2*x + 3*y + 5*z + t*(1 + x + 2*y + 3*z)'
  []
[]
[AuxVariables]
  [source]
    block = 'left right'
    family = MONOMIAL
    order = CONSTANT
  []
[]
[ICs]
  [source]
    type = FunctionIC
    variable = source
    function = source_function
  []
[]
[AuxKernels]
  [source]
    type = FunctionAux
    variable = source
    function = source_function
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
[Transfers]
  active = to_foam
  [to_foam]
    type = MultiApplibMeshToFoamTransfer
    to_multi_app = receiver
    libmesh_variable = source
    foam_variable = received
    foam_boundaries = 'left right'
    execute_on = 'initial timestep_end'
  []
  [to_foam_default]
    type = MultiApplibMeshToFoamTransfer
    to_multi_app = receiver
    libmesh_variable = source
    foam_variable = received
    # Omitted: use every physical boundary exposed by the receiver.
    execute_on = 'initial timestep_end'
  []
[]
