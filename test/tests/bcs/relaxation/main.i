[Mesh]
  type = FoamMesh
  case = foam
  foam_patch = 'left right top bottom front back'
[]

[Variables]
  [dummy]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxVariables]
  [target]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [target]
    type = ParsedAux
    variable = target
    expression = 'if(t < 0.005, 8, 16)'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Postprocessors]
  [target]
    type = ParsedPostprocessor
    expression = 'if(t < 0.005, 8, 16)'
    use_t = true
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[FoamBCs]
  [variable_left]
    type = FoamFixedValueBC
    foam_variable = T
    boundary = left
    v = target
    relaxation_factor = 0
  []
  [variable_right]
    type = FoamFixedValueBC
    foam_variable = T
    boundary = right
    v = target
    relaxation_factor = 0.5
  []
  [variable_top]
    type = FoamFixedValueBC
    foam_variable = T
    boundary = top
    v = target
    relaxation_factor = 1
  []
  [postprocessor_bottom]
    type = FoamFixedValuePostprocessorBC
    foam_variable = T
    boundary = bottom
    pp_name = target
    relaxation_factor = 0
  []
  [postprocessor_front]
    type = FoamFixedValuePostprocessorBC
    foam_variable = T
    boundary = front
    pp_name = target
    relaxation_factor = 0.5
  []
  [postprocessor_back]
    type = FoamFixedValuePostprocessorBC
    foam_variable = T
    boundary = back
    pp_name = target
    relaxation_factor = 1
  []
[]

[Problem]
  type = FoamProblem
[]

[Executioner]
  type = Transient
  end_time = 0.02
  [TimeSteppers]
    [foam]
      type = FoamTimeStepper
    []
  []
[]
