#pragma once

#include "EvaluatorTestMesh.h"
#include "MPITestFlags.h"
#include "TestUtils.h"
#include "gtest/gtest.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"
#include <functional>
#include <vector>

namespace EvaluatorTest
{
using HippoTest::expectError;
using Evaluator = LibMeshTransferEvaluator<Foam::scalar>;

/// Reference for queries with a unique nearest sample; scan independently of the KD-tree.
inline Real
expectedNearestNodeValue(const libMesh::MeshBase & mesh,
                         const Point & query,
                         const std::function<Real(const Point &)> & source_value)
{
  const libMesh::Node * best = nullptr;
  Real distance = 0.;
  for (const auto * node : mesh.node_ptr_range())
  {
    const auto d = (*node - query).norm_sq();
    if (!best || d < distance)
    {
      best = node;
      distance = d;
    }
  }
  return source_value(*best);
}

inline Real
expectedNearestCentroidValue(const libMesh::MeshBase & mesh,
                             const Point & query,
                             const std::function<Real(const libMesh::Elem &)> & value)
{
  const libMesh::Elem * best = nullptr;
  Real distance = 0.;
  for (const auto * elem : mesh.active_element_ptr_range())
  {
    const auto d = (elem->vertex_average() - query).norm_sq();
    if (!best || d < distance)
    {
      best = elem;
      distance = d;
    }
  }
  return value(*best);
}

inline void
expectValues(const Evaluator & evaluator,
             const std::vector<Point> & points,
             const std::vector<Real> & expected)
{
  Foam::Field<Foam::scalar> field(points.size(), -999.);
  evaluator.evaluate(points, field);
  EXPECT_EQ(static_cast<std::size_t>(field.size()), expected.size());
  for (std::size_t i = 0; i < expected.size(); ++i)
    EXPECT_NEAR(field[i], expected[i], 1e-10) << "request " << i;
}

class SerialFixture : public HippoTest::ErrorFixture
{
protected:
  // TIMPI's default communicator is MPI_COMM_SELF (also works without MPI).
  libMesh::Parallel::Communicator comm;
};

class MPIFixture : public HippoTest::ErrorFixture
{
protected:
  libMesh::Parallel::Communicator comm{libMesh::GLOBAL_COMM_WORLD};
};
} // namespace EvaluatorTest
