[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 3
  ny = 4
  nz = 3
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 1
[]
[Problem]
  solve = false
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
    family = LAGRANGE
    order = FIRST
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
  [to_foam]
    type = MultiApplibMeshToFoamTransfer
    to_multi_app = receiver
    libmesh_variable = source
    foam_variable = FoamVariables/received
    volume = true
    # Omitted boundary selection enables both volume and physical boundary transfers.
    execute_on = 'initial timestep_end'
  []
[]
