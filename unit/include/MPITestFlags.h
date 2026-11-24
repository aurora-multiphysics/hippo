#pragma once

#include "gtest/gtest.h"
#include "libmesh/parallel.h"

namespace HippoTest
{
inline unsigned int
mpiRankCount()
{
  const libMesh::Parallel::Communicator world(libMesh::GLOBAL_COMM_WORLD);
  return world.size();
}
} // namespace HippoTest

// Place flags first in a test body, before any collective work. GTEST_SKIP must
// return from the test itself, so these are macros rather than helper functions.
// No flag means the test can run with either one or multiple ranks.
#define REQUIRE_MPI()                                                                              \
  do                                                                                               \
  {                                                                                                \
    if (HippoTest::mpiRankCount() < 2)                                                             \
      GTEST_SKIP() << "Requires multiple MPI ranks";                                               \
  } while (false)

#define REQUIRE_MPI_RANKS(ranks)                                                                   \
  do                                                                                               \
  {                                                                                                \
    if (HippoTest::mpiRankCount() != (ranks))                                                      \
      GTEST_SKIP() << "Requires exactly " << (ranks) << " MPI ranks";                              \
  } while (false)

#define SKIP_WITH_MPI()                                                                            \
  do                                                                                               \
  {                                                                                                \
    if (HippoTest::mpiRankCount() > 1)                                                             \
      GTEST_SKIP() << "Requires a single-rank run";                                                \
  } while (false)
