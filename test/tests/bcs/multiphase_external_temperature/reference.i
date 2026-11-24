!include main.i
[Mesh]
  case := reference
[]

[FoamBCs]
  active = ''
[]

[AuxVariables]
  [heat_flux]
    family = MONOMIAL
    order = CONSTANT
  []
[]
