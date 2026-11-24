#pragma once

#include "MooseTypes.h"
#include "libmesh/parallel.h"
#include <tuple>
#include <vector>

class FEProblemBase;
class MooseVariableFieldBase;
namespace libMesh
{
class EquationSystems;
class MeshBase;
}

namespace Foam
{
template <typename T>
class Field;
}

/// Distributed source evaluation, independent of destination mesh and coordinate transforms.
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

  /// Collective: write evaluated values into the destination in input point order.
  /// Points are in the source frame. All ranks must call, even with an empty point list.
  /// Error if any requested point has no contributing source sample across all ranks.
  /// Destination size must match points; the borrowed field is never resized.
  void evaluate(const std::vector<Point> & points, Foam::Field<T> & destination) const;

protected:
  /// Look up and validate a scalar source field variable.
  static MooseVariableFieldBase & getSourceVariable(FEProblemBase & problem,
                                                    const VariableName & variable);

  /// Select the source geometry, checking that a requested displaced problem exists.
  static libMesh::EquationSystems & getSourceEquationSystems(FEProblemBase & problem,
                                                             bool use_displaced);
  static libMesh::MeshBase & getSourceMesh(FEProblemBase & problem, bool use_displaced);

  /// A zero count represents no candidate; distance and entity ID are ignored in that case.
  struct Candidate
  {
    using Tuple = std::tuple<T, unsigned, Real, dof_id_type>;

    Tuple asTuple() const { return {value, count, distance_squared, entity_id}; }

    static Candidate fromTuple(const Tuple & tuple)
    {
      const auto & [value, count, distance_squared, entity_id] = tuple;
      return {value, count, distance_squared, entity_id};
    }

    /// FE: local sum in incoming candidates, weighted average in accumulated results.
    /// Nearest-location: the individual sample value.
    T value = T(0);
    unsigned count = 0;
    Real distance_squared = 0.;
    dof_id_type entity_id = 0;
  };

  virtual std::vector<Candidate> evaluateLocal(const Point & point) const = 0;

  /// Maintain an element-count-weighted FE average from incoming local sums.
  /// Nearest-location overrides this to retain the globally nearest samples.
  virtual void accumulateContribution(std::vector<Candidate> & results,
                                      const Candidate & candidate) const;

private:
  const libMesh::Parallel::Communicator & _communicator;
};
