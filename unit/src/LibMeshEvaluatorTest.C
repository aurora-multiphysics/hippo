#include "EvaluatorTestFixture.h"

namespace EvaluatorTest
{
using LibMeshEvaluatorTest = SerialFixture;

TEST_F(LibMeshEvaluatorTest, ShapeInterpolatesInteriorOverlap)
{
  for (const bool quadratic : {false, true})
  {
    EvaluatorTestMesh source(comm, GeomType::Square, quadratic ? libMesh::SECOND : libMesh::FIRST);
    const auto source_value = [quadratic](const Point & p)
    { return 1. + 2. * p(0) + 3. * p(1) + (quadratic ? p(0) * p(0) + 2. * p(1) * p(1) : 0.); };
    source.project(source_value);
    const auto evaluator = source.getShapeEvaluator();
    const std::vector<Point> points{
        Point(0.2, 1.),  // Left internal edge.
        Point(1., 1.),   // Central vertex shared by all four elements.
        Point(1.7, 1.),  // Right internal edge.
        Point(0.3, 0.4), // Bottom-left element interior.
        Point(1.6, 1.7)  // Top-right element interior.
    };
    std::vector<Real> expected;
    for (const auto & point : points)
      expected.push_back(source_value(point));
    expectValues(*evaluator, points, expected);
  }
}

TEST_F(LibMeshEvaluatorTest, DiscontinuousOverlapAveragesOnlyContainingElements)
{
  const Real bottom_left = 2.;
  const Real bottom_right = 4.;
  const Real top_left = 8.;
  const Real top_right = 10.;
  EvaluatorTestMesh source(comm, GeomType::Square, libMesh::CONSTANT, libMesh::MONOMIAL);
  source.constants(bottom_left, bottom_right, top_left, top_right);
  const auto evaluator = source.getShapeEvaluator();

  const std::vector<Point> points{
      Point(0.2, 1.),  // Left internal edge: average bottom-left and top-left values.
      Point(1., 1.),   // Central vertex: average all four element values.
      Point(1.7, 1.),  // Right internal edge: average bottom-right and top-right values.
      Point(0.3, 0.4), // Bottom-left interior: only that element contributes.
      Point(1.6, 1.7)  // Top-right interior: only that element contributes.
  };
  const std::vector<Real> expected{(bottom_left + top_left) / 2.,
                                   (bottom_left + bottom_right + top_left + top_right) / 4.,
                                   (bottom_right + top_right) / 2.,
                                   bottom_left,
                                   top_right};
  expectValues(*evaluator, points, expected);
}

TEST_F(LibMeshEvaluatorTest, ShapeRespectsVariableBlocks)
{
  // restricted=true limits the variable to block 0 (the left half of the mesh).
  EvaluatorTestMesh source(comm, GeomType::Square, libMesh::FIRST, libMesh::LAGRANGE, true);

  const auto source_value = [](const Point & p) { return 1. + 2. * p(0) + 3. * p(1); };
  source.project(source_value);

  const auto evaluator = source.getShapeEvaluator();

  const std::vector<Point> points{Point(0.3, 0.4), Point(1., 0.4)};
  expectValues(*evaluator, points, {source_value(points[0]), source_value(points[1])});

  Foam::Field<Foam::scalar> field(2, -99.);
  Point invalid_loc(1.7, 0.4); // Inside the mesh, but outside the variable's active block.
  expectError([&]() { evaluator->evaluate({points[0], invalid_loc}, field); },
              "No source sample contributed");
}

TEST_F(LibMeshEvaluatorTest, NearestNodesAndCentroidsHandleInteriorOverlap)
{
  EvaluatorTestMesh source(comm, GeomType::Square);
  const auto source_value = [](const Point & p) { return 1. + 2. * p(0) + 3. * p(1); };
  source.project(source_value);
  const std::vector<Point> points{
      Point(0.2, 0.9), // Below the left internal edge: unique nearest node and centroid.
      Point(1.1, 1.2), // Above and right of the central node: unique nearest samples.
      Point(1.7, 1.1), // Above the right internal edge: unique nearest samples.
      Point(0.3, 0.4), // Bottom-left element interior.
      Point(1.6, 1.7)  // Top-right element interior.
  };
  std::vector<Real> nodes;
  std::vector<Real> centroids;
  for (const auto & point : points)
  {
    nodes.push_back(expectedNearestNodeValue(source.mesh, point, source_value));
    centroids.push_back(expectedNearestCentroidValue(
        source.mesh,
        point,
        [&](const libMesh::Elem & elem) { return source_value(elem.vertex_average()); }));
  }

  expectValues(*source.getNearestEvaluator("nodes"), points, nodes);
  expectValues(*source.getNearestEvaluator("variable_default"), points, nodes);
  expectValues(*source.getNearestEvaluator("centroids"), points, centroids);
}

TEST_F(LibMeshEvaluatorTest, NearestRespectsVariableBlocks)
{
  EvaluatorTestMesh source(comm, GeomType::Square, libMesh::FIRST, libMesh::LAGRANGE, true);
  const auto source_value = [](const Point & p) { return 1. + 2. * p(0) + 3. * p(1); };
  source.project(source_value);
  // The right block has geometry but no source variable
  // Evaluate at nearest location from the left block.
  expectValues(
      *source.getNearestEvaluator("nodes"), {Point(1.7, 0.4)}, {source_value(Point(1., 0.))});
  expectValues(
      *source.getNearestEvaluator("centroids"), {Point(1.7, 0.4)}, {source_value(Point(0.5, 0.5))});
}

TEST_F(LibMeshEvaluatorTest, ElementalDefaultAndExtrapolation)
{
  EvaluatorTestMesh source(comm, GeomType::Square, libMesh::CONSTANT, libMesh::MONOMIAL);
  // Evaluate at points outside the domain, it should return the nearest location that is inside the
  // domain
  source.constants(2., 4., 8., 10.);
  const std::vector<Point> points{Point(-3., -2.), Point(4., 5.), Point(0.4, 0.6)};
  expectValues(*source.getNearestEvaluator(), points, {2., 10., 2.});
  expectValues(*source.getNearestEvaluator("centroids"), points, {2., 10., 2.});
}

TEST_F(LibMeshEvaluatorTest, AveragesConfiguredNearestSamples)
{
  EvaluatorTestMesh nodes(comm, GeomType::Square);
  const auto source_value = [](const Point & p) { return 1. + 2. * p(0) + 3. * p(1); };
  nodes.project(source_value);

  // All corners of element contribute to average
  const std::vector<Point> node_points{Point(0.5, 0.5)};
  const std::vector<Real> node_expected{(source_value(Point(0., 0.)) + source_value(Point(1., 0.)) +
                                         source_value(Point(0., 1.)) +
                                         source_value(Point(1., 1.))) /
                                        4.};
  expectValues(*nodes.getNearestEvaluator("nodes", 4), node_points, node_expected);

  // Fixed-count averaging also includes samples at different distances.
  const std::vector<Point> unequal_points{Point(0.1, 0.2)};
  const std::vector<Real> unequal_expected{
      (source_value(Point(0., 0.)) + source_value(Point(0., 1.))) / 2.};
  expectValues(*nodes.getNearestEvaluator("nodes", 2), unequal_points, unequal_expected);

  EvaluatorTestMesh centroids(comm, GeomType::Square, libMesh::CONSTANT, libMesh::MONOMIAL);
  centroids.constants(2., 4., 8., 10.);
  // Four nearest centroids are all the elements in the domain: 6 for both
  const std::vector<Point> points{Point(1., 1.), Point(0.2, 0.3)};
  const std::vector<Real> expected{6., 6.};
  expectValues(*centroids.getNearestEvaluator("centroids", 4), points, expected);
  // Fewer available samples than requested: average all available samples.
  expectValues(*centroids.getNearestEvaluator("centroids", 10), points, expected);
  expectError([&]() { centroids.getNearestEvaluator("centroids", 0); },
              "num_nearest_points must be greater than zero");
}

TEST_F(LibMeshEvaluatorTest, QuadraticNodeAndCentroidSamples)
{
  EvaluatorTestMesh source(comm, GeomType::Square, libMesh::SECOND);
  const auto source_value = [](const Point & p)
  { return 1. + 2. * p(0) + 3. * p(1) + p(0) * p(0) + 2. * p(1) * p(1); };
  source.project(source_value);
  const std::vector<Point> points{Point(0.3, 0.4), Point(1.6, 1.7)};
  std::vector<Real> nodes;
  for (const auto & point : points)
    nodes.push_back(expectedNearestNodeValue(source.mesh, point, source_value));
  expectValues(*source.getNearestEvaluator("nodes"), points, nodes);
  expectValues(*source.getNearestEvaluator("centroids"),
               points,
               {source_value(Point(0.5, 0.5)), source_value(Point(1.5, 1.5))});
}

TEST_F(LibMeshEvaluatorTest, PRefinedCentroidsEvaluateEffectiveOrder)
{
  // p_level=2 makes CONSTANT MONOMIAL quadratic on QUAD4 geometry, verifying that centroid
  // sampling evaluates the full FE field rather than reading only the constant DOF.
  EvaluatorTestMesh source(comm, GeomType::Square, libMesh::CONSTANT, libMesh::MONOMIAL, false, 2);
  for (const auto * elem : source.mesh.element_ptr_range())
    ASSERT_EQ(elem->type(), libMesh::QUAD4);
  const auto source_value = [](const Point & p)
  { return 1. + 2. * p(0) + 3. * p(1) + p(0) * p(0) + 2. * p(1) * p(1); };
  source.project(source_value);
  expectValues(*source.getNearestEvaluator("centroids"),
               {Point(0.3, 0.4), Point(1.6, 1.7)},
               {source_value(Point(0.5, 0.5)), source_value(Point(1.5, 1.5))});
}

TEST_F(LibMeshEvaluatorTest, RejectsUnsupportedNodeSampling)
{
  // Node sampling does not support p-refinement and variable_default selects nodes for Lagrange
  // variables.
  EvaluatorTestMesh refined(comm, GeomType::Square, libMesh::FIRST, libMesh::LAGRANGE, false, 1);
  expectError([&]() { refined.getNearestEvaluator("nodes"); }, "p-refined source meshes");
  expectError([&]() { refined.getNearestEvaluator("variable_default"); },
              "p-refined source meshes");

  // Discontinuous elemental fields have no unique shared nodal value
  EvaluatorTestMesh discontinuous(comm, GeomType::Square, libMesh::CONSTANT, libMesh::MONOMIAL);
  expectError([&]() { discontinuous.getNearestEvaluator("nodes"); }, "discontinuous");

  // Cubic Lagrange is valid on EDGE4, but the evaluator supports only first- and second-order
  // Lagrange node sampling. This checks the evaluator restriction rather than invalid FE setup.
  EvaluatorTestMesh high_order(comm, GeomType::Line, libMesh::THIRD);
  for (const auto * elem : high_order.mesh.element_ptr_range())
    ASSERT_EQ(elem->type(), libMesh::EDGE4);
  expectError([&]() { high_order.getNearestEvaluator("nodes"); },
              "first- and second-order Lagrange");
}

TEST_F(LibMeshEvaluatorTest, ReusedEvaluatorsReadCurrentSolution)
{
  EvaluatorTestMesh source(comm, GeomType::Square);
  const auto source_value = [](const Point & p) { return 1. + 2. * p(0) + 3. * p(1); };
  source.project(source_value);

  auto shape = source.getShapeEvaluator();
  auto nodes = source.getNearestEvaluator("nodes");
  auto centroids = source.getNearestEvaluator("centroids");

  // First evaluation
  const Point query(0.3, 0.4);
  expectValues(*shape, {query}, {source_value(query)});
  expectValues(*nodes, {query}, {source_value(Point(0., 0.))});
  expectValues(*centroids, {query}, {source_value(Point(0.5, 0.5))});

  // Reevaluate without recreating evaluator
  const auto shifted_value = [&](const Point & p) { return source_value(p) + 17.; };
  source.project(shifted_value);
  expectValues(*shape, {query}, {shifted_value(query)});
  expectValues(*nodes, {query}, {shifted_value(Point(0., 0.))});
  expectValues(*centroids, {query}, {shifted_value(Point(0.5, 0.5))});
}

TEST_F(LibMeshEvaluatorTest, FixedDisplacedGeometryUsesMatchingSourceEntities)
{
  EvaluatorTestMesh source(comm, GeomType::Square);
  const auto source_value = [](const Point & p) { return 1. + 2. * p(0) + 3. * p(1); };
  source.project(source_value);
  // A geometry-only mesh copy lacks nodal DOF metadata. Initialize a matching system
  // as MOOSE does for displaced meshes, then move it before creating any locators.
  EvaluatorTestMesh geometry(comm, GeomType::Square);
  for (auto * node : geometry.mesh.node_ptr_range())
    (*node)(0) += 10.;
  // Query on shifted mesh should evaluate to original
  const Point query(10.3, 0.4);

  expectValues(
      *source.getShapeEvaluator(&geometry.equations), {query}, {source_value(Point(0.3, 0.4))});

  // Nearest node is at (0. 0.)
  expectValues(*source.getNearestEvaluator("nodes", 1, &geometry.mesh),
               {query},
               {source_value(Point(0., 0.))});
  // Nearest centroid is at (0.5 0.5)
  expectValues(*source.getNearestEvaluator("centroids", 1, &geometry.mesh),
               {query},
               {source_value(Point(0.5, 0.5))});
}

TEST_F(LibMeshEvaluatorTest, EmptyRequestsAndInvalidDestinationSizes)
{
  EvaluatorTestMesh source(comm, GeomType::Square);
  const auto source_value = [](const Point & p) { return 1. + 2. * p(0) + 3. * p(1); };
  source.project(source_value);
  auto shape = source.getShapeEvaluator();
  auto nearest = source.getNearestEvaluator();

  std::set<Evaluator *> evaluators{static_cast<Evaluator *>(shape.get()),
                                   static_cast<Evaluator *>(nearest.get())};
  for (const Evaluator * evaluator : evaluators)
  {
    // Check empty points and fields does nothing
    Foam::Field<Foam::scalar> empty;
    evaluator->evaluate({}, empty);
    EXPECT_EQ(empty.size(), 0);

    // Check different sizes of points and field causes error.
    Foam::Field<Foam::scalar> field(2, -99.);
    expectError([&]() { evaluator->evaluate({Point(0.3, 0.4)}, field); }, "Destination field size");
  }
}

TEST_F(LibMeshEvaluatorTest, MissingSourcesRaiseErrors)
{
  EvaluatorTestMesh source(comm, GeomType::Square);
  const auto source_value = [](const Point & p) { return 1. + 2. * p(0) + 3. * p(1); };
  source.project(source_value);
  auto shape = source.getShapeEvaluator();
  Foam::Field<Foam::scalar> field(2, -99.);
  expectError([&]() { shape->evaluate({Point(0.3, 0.4), Point(3., 3.)}, field); },
              "No source sample contributed");

  // Variable not defined on any blocks so even points on the mesh will raise error
  EvaluatorTestMesh absent(comm,
                           GeomType::Square,
                           libMesh::FIRST,
                           libMesh::LAGRANGE,
                           false,
                           0,
                           Ownership::Default,
                           true);
  for (const auto & mode : {"nodes", "centroids"})
  {
    auto evaluator = absent.getNearestEvaluator(mode);
    expectError([&]() { evaluator->evaluate({Point(0.3, 0.4), Point(1.3, 0.4)}, field); },
                "No source sample contributed");
  }
}
} // namespace EvaluatorTest
