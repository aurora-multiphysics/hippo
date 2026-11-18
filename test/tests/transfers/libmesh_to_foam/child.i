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
  # Both objects borrow the same native field through separate accessors; one also mirrors it.
  [received_alias]
    type = FoamVariableField
    foam_variable = transferField
    create_aux_variable = false
  []
  [received]
    type = FoamVariableField
    foam_variable = transferField
    create_aux_variable = true
  []
[]
