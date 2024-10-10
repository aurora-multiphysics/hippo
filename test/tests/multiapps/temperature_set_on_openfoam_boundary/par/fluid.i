[Mesh]
  type = FoamMesh
  foam_args = '-case buoyantCavity -parallel'
  foam_patch = 'patch2 patch4'
  dim=2
[]

[Variables]
  [Dummy]
    initial_condition = 3.14
  []
[]

[AuxVariables]
  [T]
    initial_condition = 111
    family = MONOMIAL
    order = CONSTANT
  []
  [foam_T]
    initial_condition = 999
    family = MONOMIAL
    order = CONSTANT
  []
  [foam_hf]
    initial_condition = -999
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Problem]
  type=BuoyantFoamProblem
  temp = T
  foam_temp = foam_T
  foam_heat_flux = foam_hf
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 0.1
  dt = 0.1

  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  [TimeStepper]
    type = FoamTimeStepper
  []
[]

[Outputs]
  exodus = true
[]
