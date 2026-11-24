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

[Executioner]
  type = Transient
  dt = 1
  end_time = 4
[]

[Functions]
  [value]
    type = ParsedFunction
    expression = '100 + (1+t)*(x+y+z)'
  []
  [gradient]
    type = ParsedFunction
    expression = '-(1+t)'
  []
  [flux]
    type = ParsedFunction
    expression = '2*(1+t)'
  []
[]

[AuxVariables]
  [value]
    family = LAGRANGE
    order = FIRST
  []
  [gradient]
    family = LAGRANGE
    order = FIRST
  []
  [flux]
    family = LAGRANGE
    order = FIRST
  []
[]

# INITIAL MultiApp transfers run before INITIAL AuxKernels, so initialize sources with ICs.
[ICs]
  [value]
    type = FunctionIC
    variable = value
    function = value
  []
  [gradient]
    type = FunctionIC
    variable = gradient
    function = gradient
  []
  [flux]
    type = FunctionIC
    variable = flux
    function = flux
  []
[]

[AuxKernels]
  [value]
    type = FunctionAux
    variable = value
    function = value
    execute_on = timestep_end
  []
  [gradient]
    type = FunctionAux
    variable = gradient
    function = gradient
    execute_on = timestep_end
  []
  [flux]
    type = FunctionAux
    variable = flux
    function = flux
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
  [to_value]
    type = MultiApplibMeshToFoamTransfer
    to_multi_app = receiver
    libmesh_variable = value
    foam_variable = FoamBCs/prescribed_value
    execute_on = 'initial timestep_end'
  []
  [to_gradient]
    type = MultiApplibMeshToFoamTransfer
    to_multi_app = receiver
    libmesh_variable = gradient
    foam_variable = FoamBCs/prescribed_gradient
    execute_on = 'initial timestep_end'
  []
  [to_flux]
    type = MultiApplibMeshToFoamTransfer
    to_multi_app = receiver
    libmesh_variable = flux
    foam_variable = FoamBCs/prescribed_flux
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  file_base = direct_transfer
[]
