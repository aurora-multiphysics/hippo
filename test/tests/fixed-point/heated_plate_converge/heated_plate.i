[Mesh]
    [solid_shape]
        # This mesh is set up to have a 1-1 correspondence of cell edges at the
        # interface boundary.
        type = CartesianMeshGenerator
        dim = 3
        # The OpenFOAM mesh uses 'simpleGrading' along the interface, which
        # sets the 'expansion ratio' (the width of the final cell over the
        # width of the first) in each direction.
        # In this example the expansion ratio for the interface in the x
        # direction is 5. The interface has a length of 1m. Hence we get the
        # following properties:
        #   cell-to-cell expansion ratio: 1.010109749
        #   start cell width:             0.002495896054 m
        #   end cell width:               0.01247948027 m
        #
        # The dx values can be computed using the Python snippet:
        #   cc_expansion_ratio = 1.010109749
        #   width_0 = 0.002495896054
        #   num_cells = 161
        #   dx = width_0 * cc_expansion_ratio**np.arange(num_cells)
        dx = '0.002495896054 0.00252112893663603 0.0025466169173820573 0.0025723625752159434 0.0025983685151883696 0.002624637368686427 0.0026511717936998675 0.0026779744750900524 0.0027050481248616196 0.0027323954824368907 0.0027600193149330617 0.0027879224174421867 0.0028161076133140003 0.002844577754441594 0.0028733357215499815 0.0029023844244875857 0.0029317268025206645 0.002961365824630721 0.0029913044898149154 0.003021545827389517 0.0030520928972964222 0.0030829487904127715 0.003114116628863698 0.003145599566338236 0.003177400788408424 0.0032095235128516354 0.0032419709899761634 0.003274746502950104 0.003307853368133557 0.0033412949354141917 0.0033750745885462005 0.0034091957454926805 0.003443661858771479 0.003478476415804532 0.003513642939270735 0.0035491649874623845 0.0035850461546452174 0.0036212900714220958 0.0036579004051003647 0.0036948808600629273 0.003732235178143068 0.003769967139003064 0.003808080560516633 0.003846579299155235 0.0038854672503782902 0.003924748349027335 0.0039644265697241655 0.004004505927273008 0.00404499047706675 0.004085884315497284 0.004127191580369998 0.0041689164513224524 0.004211063150247293 0.0042536359417194415 0.004296639133427604 0.004340077076610134 0.0043839541664953165 0.004428274842746088 0.004473043589909265 0.004518264937869307 0.004563943462306665 0.004610083785160777 0.004656690575097722 0.004703768547982625 0.004751322467356824 0.0047993571449198615 0.0048478774410163575 0.004896888265127795 0.004946394576369283 0.004996401383991336 0.005046913747886741 0.0050979367791025255 0.00514947564035712 0.005201535546562744 0.005254121765353071 0.005307239617616228 0.005360894478033183 0.0054150917756215855 0.005469836994285083 0.005525135673368219 0.005580993408216917 0.0056374158507446455 0.005694408710004295 0.0057519777527658515 0.005810128804099898 0.005868867747967017 0.005928200527813159 0.005988133147171017 0.006048671670267496 0.006109822222637311 0.006171590991742796 0.006233984227599977 0.006297008243410971 0.006360669416202786 0.006424974187472572 0.006489929063839399 0.006555540617702619 0.006621815487906898 0.006688760380413949 0.006756382068981078 0.006824687395846577 0.006893683272422049 0.006963376679991734 0.007033774670418903 0.007104884366859396 0.007176712964482368 0.00724926773119833 0.007322556008394545 0.007396585211677856 0.00747136283162503 0.007546896434540687 0.007623193663222888 0.007700262237736463 0.007778109956194155 0.007856744695545679 0.007936174412374727 0.008016407143704057 0.008097451007808712 0.008179314205037454 0.008262005018642517 0.008345531815617733 0.008429903047545142 0.008515127251450158 0.008601213050665378 0.00868816915570313 0.00877600436513683 0.008864727566491266 0.008954347737141873 0.009044873945223094 0.00913631535054594 0.009228681205524806 0.009321980856113679 0.009416223742751792 0.009511419401318853 0.009607577464099917 0.009704707660760023 0.009802819819328684 0.009901923867194322 0.010002029832108765 0.010103147843201895 0.010205288132006558 0.010308461033493824 0.010412676987118724 0.010517946537876572 0.010624280337369922 0.010731689144886368 0.010840183828487192 0.010949775366107055 0.01106047484666478 0.011172293471185375 0.011285242553933395 0.011399333523557782 0.011514577924248235 0.011630987416903327 0.011748573780310377 0.011867348912337294 0.011987324831136447 0.012108513676360703 0.012230927710391777 0.012354579319580982 0.012479481015502534'
        dy = '0.015625 0.015625 0.015625 0.015625 0.015625 0.015625 0.015625 0.015625 0.015625 0.015625 0.015625 0.015625 0.015625 0.015625 0.015625 0.015625'  # 16 cells
        dz = '0.4'
    []
    [solid_boundary_renamed]
        type = RenameBoundaryGenerator
        input = solid_shape
        old_boundary = 'top bottom'
        new_boundary = 'solid_top solid_bottom'
    []
    [solid]
        type = TransformGenerator
        input = solid_boundary_renamed
        transform = TRANSLATE
        vector_value = '0 -0.25 0'
    []
[]

[MultiApps]
    [hippo]
        type = TransientMultiApp
        app_type = hippoApp
        execute_on = timestep_begin
        input_files = 'fluid.i'
    []
[]

[Transfers]
    [wall_temperature_from_fluid]
        type = MultiAppGeometricInterpolationTransfer
        source_variable = fluid_wall_temp
        from_multi_app = hippo
        variable = fluid_wall_temperature
        execute_on = TIMESTEP_BEGIN
    []

    [heat_flux_to_fluid]
        type = MultiAppGeometricInterpolationTransfer
        source_variable = wall_heat_flux
        to_multi_app = hippo
        variable = solid_heat_flux
        execute_on = TIMESTEP_BEGIN
    []
[]

[Variables]
    [temp]
        family = LAGRANGE
        order = FIRST
        initial_condition = 310
    []
[]

[AuxVariables]
    [fluid_wall_temperature]
        family = LAGRANGE
        order = FIRST
        initial_condition = 0
    []
    [wall_heat_flux]
        family = MONOMIAL
        order = CONSTANT
        initial_condition = 0
    []
[]

[Kernels]
    [heat-conduction]
        type = HeatConduction
        variable = temp
    []
    [heat-conduction-dt]
        type = HeatConductionTimeDerivative
        variable = temp
    []
[]

[AuxKernels]
    [heat_flux_aux]
        type = HeatFluxAux
        variable = wall_heat_flux
        T = temp
        boundary = 'solid_top'
        execute_on = 'MULTIAPP_FIXED_POINT_BEGIN TIMESTEP_END'
    []
[]

[BCs]
    [fixed_temp]
        type = DirichletBC
        variable = temp
        boundary = solid_bottom
        value = 310
    []

    [fluid_interface]
        type = MatchedValueBC
        variable = temp
        boundary = solid_top
        v = fluid_wall_temperature
    []
[]

[Materials]
    # The example specifies that the thermal diffusivity of the solid should
    # be α = 1 m2/s, and the thermal conductivity should be k = 100 W/(m.K).
    #
    # We know α = k/(Cp.ρ), where k is thermal conductivity, Cp is specific
    # heat capacity, and ρ is density.
    #
    # Hence we require that Cp.ρ = k = 100.
    [thermal-conduction]
        type = HeatConductionMaterial
        thermal_conductivity = 100.0  # W/(m.K)
        specific_heat = 0.5  # J/(kg.K)
    []
    [thermal-density]
        type = GenericConstantMaterial
        prop_names  = 'density'
        prop_values = 200.0  # kg/m3
    []
  []

[Executioner]
    type = Transient
    start_time = 0
    end_time = 1
    dt = 0.025
    fixed_point_max_its=20
    fixed_point_min_its=1
    fixed_point_rel_tol=1e-3
    fixed_point_abs_tol=1e-8
    solve_type = 'NEWTON'
    petsc_options = '-snes_ksp_ew'
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'
    nl_abs_tol = 1e-7
    l_tol = 1e-8
    nl_rel_tol = 1e-8
    l_abs_tol = 1e-9
[]

[Outputs]
    exodus = true
[]
