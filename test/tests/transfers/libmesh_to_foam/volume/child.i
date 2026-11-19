[Mesh]
  type = FoamMesh
  case = child_case
  foam_patch = 'left right'
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
[FoamVariables]
  # Used by the missing-internal-storage error case.
  [boundary_only]
    type = FoamBoundaryScalarVariable
    boundary = 'left right'
  []
  [received]
    type = FoamVariableField
    foam_variable = transferField
    create_aux_variable = true
  []
[]
