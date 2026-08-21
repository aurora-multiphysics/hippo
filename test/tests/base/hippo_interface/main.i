[Mesh]
    type = FoamMesh
    case = 'foam'
    foam_patch = 'left right bottom top back front'
[]

[Kernels]
    [dummy]
        type = NullKernel
        variable = dummy
    []
[]

[Postprocessors]
    [p1]
        type = FoamSideAverageValue
        foam_variable=T
        boundary=left
    []
[]

[Variables]
    [dummy]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 999
    []
[]

[Problem]
    type = FoamProblem
    # Take the boundary temperature from OpenFOAM and set it on the MOOSE mesh.
[]

[Executioner]
    type = Transient
    end_time = 0.01
    dt = 0.01
[]

[Outputs]
    exodus = true
[]
