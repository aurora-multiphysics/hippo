[Mesh]
    type = FoamMesh
    case = 'foam'
    foam_patch = 'left right bottom top back front'
[]

[FoamBCs]
    [temp1]
        type=FoamFixedValueBC
        foam_variable = T
        boundary = 'left right top' # test boundary restrictions
    []
    [temp2]
        type=FoamFixedValueBC
        foam_variable = T
        boundary = 'bottom front back' # test boundary restrictions
    []
[]

[Problem]
    type = FoamProblem
[]

[Executioner]
    type = Transient
    end_time = 0.32
    [TimeSteppers]
        [foam]
            type = FoamTimeStepper
        []
    []
[]
