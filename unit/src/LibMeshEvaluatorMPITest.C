#include "EvaluatorTestFixture.h"
#include <algorithm>
#include <utility>

namespace EvaluatorTest
{
using LibMeshEvaluatorMPITest = MPIFixture;

TEST_F(LibMeshEvaluatorMPITest, InteriorOverlapAndUnequalContributions)
{
  REQUIRE_MPI_RANKS(2);

  const Real bottom_left = 2.;
  const Real bottom_right = 4.;
  const Real top_left = 8.;
  const Real top_right = 10.;
  const std::vector<Point> sample_points{
      Point(0.2, 1.),  // Left internal edge: average bottom-left and top-left values.
      Point(1., 1.),   // Central vertex: average all four element values.
      Point(1.7, 1.),  // Right internal edge: average bottom-right and top-right values.
      Point(0.3, 0.4), // Bottom-left interior: only that element contributes.
      Point(1.6, 1.7)  // Top-right interior: only that element contributes.
  };
  const std::vector<Real> sample_expected{(bottom_left + top_left) / 2.,
                                          (bottom_left + bottom_right + top_left + top_right) / 4.,
                                          (bottom_right + top_right) / 2.,
                                          bottom_left,
                                          top_right};

  // Only rank 0 requests values. Swap the 3:1 element ownership between ranks.
  for (const auto ownership : {Ownership::ThreeOnRankZero, Ownership::ThreeOnRankOne})
  {
    EvaluatorTestMesh source(
        comm, GeomType::Square, libMesh::CONSTANT, libMesh::MONOMIAL, false, 0, ownership);
    source.constants(bottom_left, bottom_right, top_left, top_right);
    auto shape = source.getShapeEvaluator();
    auto nearest = source.getNearestEvaluator("centroids");

    // Only rank 0 has destinations. Each rank contributes only its owned elements.
    const auto points = comm.rank() == 0 ? sample_points : std::vector<Point>{};
    const auto expected = comm.rank() == 0 ? sample_expected : std::vector<Real>{};
    expectValues(*shape, points, expected);

    // Reference value from the constants assigned to each quadrant.
    const auto element_value = [&](const libMesh::Elem & elem)
    {
      const auto element_center = elem.vertex_average();
      if (element_center(1) > 1.) // Element is in the upper half.
        return element_center(0) > 1. ? top_right : top_left;
      return element_center(0) > 1. ? bottom_right : bottom_left;
    };

    const std::vector<Point> nearest_samples{
        Point(0.2, 0.9), Point(1.1, 1.2), Point(1.7, 1.1), Point(0.3, 0.4), Point(1.6, 1.7)};
    const auto nearest_points = comm.rank() == 0 ? nearest_samples : std::vector<Point>{};

    // Rank 1 has no requests or expected values, but still participates in evaluation.
    std::vector<Real> nearest_expected;
    for (const auto & point : nearest_points)
      nearest_expected.push_back(expectedNearestCentroidValue(source.mesh, point, element_value));

    expectValues(*nearest, nearest_points, nearest_expected);

    // Verify the 3:1 ownership needed to detect incorrect equal weighting of rank averages.
    unsigned int local_elements = 0;
    for (const auto * elem : source.mesh.active_local_element_ptr_range())
    {
      (void)elem;
      ++local_elements;
    }
    EXPECT_EQ(local_elements,
              ownership == Ownership::ThreeOnRankZero ? (comm.rank() == 0 ? 3u : 1u)
                                                      : (comm.rank() == 0 ? 1u : 3u));
  }
}

TEST_F(LibMeshEvaluatorMPITest, LinearInteriorOverlapAndRemoteNearestNodes)
{
  REQUIRE_MPI_RANKS(2);

  for (const auto ownership : {Ownership::ThreeOnRankZero, Ownership::ThreeOnRankOne})
  {
    EvaluatorTestMesh source(
        comm, GeomType::Square, libMesh::FIRST, libMesh::LAGRANGE, false, 0, ownership);
    const auto source_value = [](const Point & p) { return 1. + 2. * p(0) + 3. * p(1); };
    source.project(source_value);
    auto shape = source.getShapeEvaluator();
    auto nearest = source.getNearestEvaluator();

    // Only rank 0 requests values; rank 1 still supplies its owned source samples.
    auto points = std::vector<Point>{};
    if (comm.rank() == 0)
      points = {Point(0.2, 1.), Point(1., 1.), Point(1.7, 1.), Point(0.3, 0.4), Point(1.6, 1.7)};

    std::vector<Real> interpolated;
    std::vector<Real> sampled;
    for (const auto & point : points)
    {
      interpolated.push_back(source_value(point));
      sampled.push_back(expectedNearestNodeValue(source.mesh, point, source_value));
    }
    expectValues(*shape, points, interpolated);
    expectValues(*nearest, points, sampled);
  }
}

TEST_F(LibMeshEvaluatorMPITest, GlobalNearestSamplesArePartitionIndependent)
{
  REQUIRE_MPI_RANKS(2);

  const Real bottom_left = 2.;
  const Real bottom_right = 4.;
  const Real top_left = 8.;
  const Real top_right = 10.;
  const auto element_value = [&](const libMesh::Elem & elem)
  {
    const auto element_center = elem.vertex_average();
    if (element_center(1) > 1.)
      return element_center(0) > 1. ? top_right : top_left;
    return element_center(0) > 1. ? bottom_right : bottom_left;
  };

  for (const auto ownership :
       {Ownership::ThreeOnRankZero, Ownership::ThreeOnRankOne, Ownership::AllOnRankZero})
  {
    EvaluatorTestMesh source(
        comm, GeomType::Square, libMesh::CONSTANT, libMesh::MONOMIAL, false, 0, ownership);
    source.constants(bottom_left, bottom_right, top_left, top_right);
    // All centroids tie at the center; element ID resolves the cutoff for N=1 and N=2.
    // Other queries exercise unequal distances and candidates split across ranks.
    const std::vector<Point> points{Point(1., 1.), Point(0.5, 0.5), Point(1.6, 1.7)};
    for (const unsigned int count : {1u, 2u, 4u, 10u})
    {
      std::vector<Real> expected;
      for (const auto & point : points)
      {
        // Scan the full replicated mesh, sorting by squared distance, then element ID for ties.
        std::vector<std::pair<Real, dof_id_type>> samples;
        for (const auto * elem : source.mesh.active_element_ptr_range())
          samples.emplace_back((elem->vertex_average() - point).norm_sq(), elem->id());
        std::sort(samples.begin(), samples.end());

        // Average the requested number of global samples, or all samples if fewer exist.
        Real sum = 0.;
        const auto size = std::min<std::size_t>(count, samples.size());
        for (std::size_t i = 0; i < size; ++i)
        {
          sum += element_value(source.mesh.elem_ref(samples[i].second));
        }
        expected.push_back(sum / size);
      }
      expectValues(*source.getNearestEvaluator("centroids", count), points, expected);
    }

    // Repeat the global reference calculation for nodes, including ties at (0.5, 0.5).
    EvaluatorTestMesh nodes(
        comm, GeomType::Square, libMesh::FIRST, libMesh::LAGRANGE, false, 0, ownership);
    const auto source_value = [](const Point & p) { return 1. + 2. * p(0) + 3. * p(1); };
    nodes.project(source_value);

    const std::vector<Point> node_points{Point(0.5, 0.5), Point(1.6, 1.7)};
    for (const unsigned int count : {1u, 2u, 4u, 20u})
    {
      std::vector<Real> expected;
      for (const auto & point : node_points)
      {
        std::vector<std::pair<Real, dof_id_type>> samples;
        for (const auto * node : nodes.mesh.node_ptr_range())
          samples.emplace_back((*node - point).norm_sq(), node->id());
        std::sort(samples.begin(), samples.end());

        Real sum = 0.;
        const auto size = std::min<std::size_t>(count, samples.size());
        for (std::size_t i = 0; i < size; ++i)
          sum += source_value(nodes.mesh.node_ref(samples[i].second));
        expected.push_back(sum / size);
      }
      expectValues(*nodes.getNearestEvaluator("nodes", count), node_points, expected);
    }
  }
}

TEST_F(LibMeshEvaluatorMPITest, EmptySourceRankCanRequestRemoteValues)
{
  REQUIRE_MPI_RANKS(2);
  // Rank 0 owns all source samples; only rank 1 requests destination values.
  EvaluatorTestMesh source(comm,
                           GeomType::Square,
                           libMesh::FIRST,
                           libMesh::LAGRANGE,
                           false,
                           0,
                           Ownership::AllOnRankZero);

  const auto source_value = [](const Point & p) { return 1. + 2. * p(0) + 3. * p(1); };
  source.project(source_value);

  auto shape = source.getShapeEvaluator();
  auto nodes = source.getNearestEvaluator("nodes");
  auto centroids = source.getNearestEvaluator("centroids");

  // Rank 1 must obtain these values from rank 0 despite owning no source samples.
  const auto points = comm.rank() == 1 ? std::vector<Point>{Point(0.3, 0.4)} : std::vector<Point>{};

  expectValues(*shape, points, comm.rank() == 1 ? std::vector<Real>{2.8} : std::vector<Real>{});
  expectValues(*nodes, points, comm.rank() == 1 ? std::vector<Real>{1.} : std::vector<Real>{});
  expectValues(*centroids, points, comm.rank() == 1 ? std::vector<Real>{3.5} : std::vector<Real>{});
}

TEST_F(LibMeshEvaluatorMPITest, MissingPointErrorsOnEveryRank)
{
  REQUIRE_MPI_RANKS(2);

  EvaluatorTestMesh source(comm,
                           GeomType::Square,
                           libMesh::FIRST,
                           libMesh::LAGRANGE,
                           false,
                           0,
                           Ownership::ThreeOnRankZero);

  const auto source_value = [](const Point & p) { return 1. + 2. * p(0) + 3. * p(1); };
  source.project(source_value);
  auto evaluator = source.getShapeEvaluator();

  // Rank 0 requests (3, 3) outside the domain; both ranks must error despite rank 1's valid point.
  const std::vector<Point> points{comm.rank() == 0 ? Point(3., 3.) : Point(0.3, 0.4)};
  Foam::Field<Foam::scalar> field(1, -99.);
  expectError([&]() { evaluator->evaluate(points, field); }, "No source sample contributed");
}

TEST_F(LibMeshEvaluatorMPITest, SizeMismatchErrorsOnEveryRank)
{
  REQUIRE_MPI_RANKS(2);

  EvaluatorTestMesh source(comm,
                           GeomType::Square,
                           libMesh::FIRST,
                           libMesh::LAGRANGE,
                           false,
                           0,
                           Ownership::ThreeOnRankZero);
  const auto source_value = [](const Point & p) { return 1. + 2. * p(0) + 3. * p(1); };
  source.project(source_value);
  auto evaluator = source.getNearestEvaluator();

  // Only rank 0 has a size mismatch (two entries for one point); both ranks must error.
  Foam::Field<Foam::scalar> field(comm.rank() == 0 ? 2 : 1, -99.);
  expectError([&]() { evaluator->evaluate({Point(0.3, 0.4)}, field); }, "Destination field size");
}

TEST_F(LibMeshEvaluatorMPITest, GloballyEmptySamplesErrorOnEveryRank)
{
  REQUIRE_MPI_RANKS(2);

  // no_active_blocks=true restricts the variable to nonexistent block 99.
  // Both ranks have mesh geometry, but neither has eligible variable samples.
  EvaluatorTestMesh source(comm,
                           GeomType::Square,
                           libMesh::CONSTANT,
                           libMesh::MONOMIAL,
                           false,
                           0,
                           Ownership::ThreeOnRankZero,
                           true);
  auto evaluator = source.getNearestEvaluator("centroids");
  // Rank 1 must also report the global failure even though it requests no values.
  const auto points = comm.rank() == 0 ? std::vector<Point>{Point(0.3, 0.4)} : std::vector<Point>{};
  Foam::Field<Foam::scalar> field(points.size(), -99.);
  expectError([&]() { evaluator->evaluate(points, field); }, "No source sample contributed");
}
} // namespace EvaluatorTest
