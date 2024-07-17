[Mesh]
    # The part of the mesh to the left of the heated plate (where +-ve x points right)
    [left]
        type = GeneratedMeshGenerator
        dim = 3
        nx = 50
        ny = 75
        nz = 1
        xmin = 0.0
        xmax = 0.5
        ymin = 0.25
        ymax = 0.75
        zmin = 0
        zmax = 0.5
        elem_type = HEX8
        boundary_name_prefix = solid_left
    []
    # The part of the mesh whose bottom patch interfaces the solid domain
    [fluid_interface]
        type = GeneratedMeshGenerator
        dim = 3
        nx = 100
        ny = 75
        nz = 1
        xmin = 0.5
        xmax = 1.5
        ymin = 0.25
        ymax = 0.75
        zmin = 0
        zmax = 0.5
        elem_type = HEX8
        boundary_name_prefix = solid_fluid_interface
    []
    # The part of the mesh to the right of the heated plate
    [right]
        type = GeneratedMeshGenerator
        dim = 3
        nx = 200
        ny = 75
        nz = 1
        xmin = 1.5
        xmax = 3.5
        ymin = 0.25
        ymax = 0.75
        zmin = 0
        zmax = 0.5
        elem_type = HEX8
        boundary_name_prefix = solid_right
    []
    [solid]
        type = StitchedMeshGenerator
        inputs = 'left fluid_interface right'
        clear_stitched_boundary_ids = true
        stitch_boundaries_pairs = 'solid_left_right solid_fluid_interface_left;
                                   solid_fluid_interface_right solid_right_left '
        parallel_type = 'replicated'
      []
[]
