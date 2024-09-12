[Mesh]
    [fluid]
        type = GeneratedMeshGenerator
        dim = 3
        nx = 10
        ny = 5
        nz = 5
        xmin = 0
        xmax = 0.02
        ymin = 0.01
        ymax = 0.02
        zmin = 0
        zmax = 0.01
        elem_type = HEX8
        boundary_name_prefix = fluid
    []
[]
