#pragma once

#include "MooseTypes.h"
#include "libmesh/parallel.h"
#include <utility>
#include <vector>

/// Distributed source evaluation, independent of destination fields and coordinate transforms.
/// The distributed implementation is currently instantiated for Foam::scalar only.
template <typename T>
class LibMeshTransferEvaluator
{
public:
  /// The communicator must outlive this evaluator and include all participating source ranks.
  explicit LibMeshTransferEvaluator(const libMesh::Parallel::Communicator & communicator)
    : _communicator(communicator)
  {
  }

  virtual ~LibMeshTransferEvaluator() = default;

  /// Collective: return averaged values in input point order.
  /// Points are in the source frame. All ranks must call, even with an empty point list.
  /// Error if any requested point has no contributing source element across all ranks.
  std::vector<T> evaluate(const std::vector<Point> & points) const;

protected:
  /// Return the local value sum and owned-element count, or {0, 0} if none contribute.
  virtual std::pair<T, unsigned> evaluateLocal(const Point & point) const = 0;

private:
  const libMesh::Parallel::Communicator & _communicator;
};
