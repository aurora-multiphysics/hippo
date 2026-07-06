[Mesh]
  type = FoamMesh
  case = 'fluid-openfoam'
  foam_patch = 'interface'
[]

[Variables]
  [Dummy]
    initial_condition = 3.14
  []
[]

[Problem]
  type=FoamProblem
[]

[Executioner]
  type = Transient

  end_time=0.51
  [TimeStepper]
    type = FoamTimeStepper
  []
[]

[Outputs]
  exodus = true
[]
