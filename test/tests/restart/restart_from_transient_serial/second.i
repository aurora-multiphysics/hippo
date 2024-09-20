[Mesh]
  type = FoamMesh
  foam_args = '-case buoyantCavity'
  foam_patch = 'topAndBottom frontAndBack'
  dim=2
[]

[Variables]
  [Dummy]
  []
[]

[AuxVariables]
  [T]
  []

  [foam_T]
  []

  [foam_hf]
  []
[]

[Problem]
  type = FoamProblem
  temp = T
  foam_temp = foam_T
  foam_heat_flux = foam_hf
  restart_file_base = first_out_cp/LATEST
[]

[Executioner]
  type = Transient
  start_time = 1
  end_time = 2
  dt = 0.25
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  [TimeStepper]
    type = FoamTimeStepper
  []
[]

[Outputs]
  exodus = true
[]
