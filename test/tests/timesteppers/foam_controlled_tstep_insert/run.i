[Mesh]
  type = FoamMesh
  case = 'buoyantCavity'
  foam_patch = 'topAndBottom frontAndBack'
[]

[Variables]
  [Dummy]
    initial_condition = 3.14
  []
[]

[Kernels]
  [null]
    type=NullKernel
    variable = Dummy
  []
[]

[MultiApps]
  [hippo]
    type = TransientMultiApp
    app_type = hippoApp
    execute_on = timestep_begin
    input_files = 'fluid.i'
    sub_cycling = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  end_time=3.
  dt = 1.

[]

[Outputs]
  exodus = true
[]
