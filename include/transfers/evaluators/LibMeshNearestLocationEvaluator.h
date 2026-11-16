#pragma once

#include "fvCFD_moose.h"
#include "LibMeshTransferEvaluator.h"
#include "KDTree.h"
#include <cstddef>
#include <memory>

class FEProblemBase;
class MooseEnum;
namespace libMesh
{
class System;
class Elem;
class MeshBase;
}

/// Nearest owned node or element vertex-average sample on a fixed source mesh.
template <typename T>
class LibMeshNearestLocationEvaluator : public LibMeshTransferEvaluator<T>
{
public:
  /// Source geometry, topology, ownership, and variable support must remain fixed.
  /// The source problem, solution, and communicator must outlive this evaluator.
  LibMeshNearestLocationEvaluator(FEProblemBase & problem,
                                  const VariableName & variable,
                                  bool use_displaced,
                                  const libMesh::Parallel::Communicator & communicator,
                                  const MooseEnum & source_type);

  /// Direct libMesh entry point; source_is_nodal supplies variable_default semantics.
  /// Geometry must share source entity IDs/topology. All dependencies must outlive this object.
  LibMeshNearestLocationEvaluator(libMesh::System & system,
                                  unsigned int variable_number,
                                  libMesh::MeshBase & geometry,
                                  const libMesh::Parallel::Communicator & communicator,
                                  const MooseEnum & source_type,
                                  bool source_is_nodal,
                                  unsigned int max_leaf_size);

protected:
  using LibMeshTransferEvaluator<T>::getSourceVariable;
  using LibMeshTransferEvaluator<T>::getSourceMesh;
  using Candidate = typename LibMeshTransferEvaluator<T>::Candidate;
  Candidate evaluateLocal(const Point & point) const override;
  void accumulateContribution(Candidate & result, const Candidate & candidate) const override;

private:
  /// Read the selected local sample and attach its search metadata.
  Candidate sampleCandidate(std::size_t sample_index, Real distance_squared) const;

  struct Sample
  {
    dof_id_type entity_id;
    dof_id_type dof;
    const libMesh::Elem * elem;
    Point value_point;
  };

  libMesh::System * _source_system = nullptr;
  unsigned int _variable_number = 0;
  std::vector<Sample> _samples;
  std::vector<Point> _points;
  // Declared last so the tree is destroyed before the points it references.
  std::unique_ptr<KDTree> _tree;
};

extern template class LibMeshNearestLocationEvaluator<Foam::scalar>;
