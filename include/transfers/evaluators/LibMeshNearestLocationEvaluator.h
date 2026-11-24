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

/// Fixed-count nearest sampling of owned nodes or element vertex averages on a fixed mesh.
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
                                  const MooseEnum & source_type,
                                  unsigned int num_nearest_points = 1);

  /// Direct libMesh entry point; source_is_nodal supplies variable_default semantics.
  /// Geometry must share source entity IDs/topology. All dependencies must outlive this object.
  LibMeshNearestLocationEvaluator(libMesh::System & system,
                                  unsigned int variable_number,
                                  libMesh::MeshBase & geometry,
                                  const libMesh::Parallel::Communicator & communicator,
                                  const MooseEnum & source_type,
                                  bool source_is_nodal,
                                  unsigned int max_leaf_size,
                                  unsigned int num_nearest_points = 1);

protected:
  using LibMeshTransferEvaluator<T>::getSourceVariable;
  using LibMeshTransferEvaluator<T>::getSourceMesh;
  using Candidate = typename LibMeshTransferEvaluator<T>::Candidate;
  std::vector<Candidate> evaluateLocal(const Point & point) const override;
  void accumulateContribution(std::vector<Candidate> & results,
                              const Candidate & candidate) const override;

private:
  /// Read a selected local sample without synchronizing the solution.
  T sampleValue(std::size_t sample_index) const;

  struct Sample
  {
    dof_id_type entity_id;
    dof_id_type dof;
    const libMesh::Elem * elem;
    Point value_point;
  };

  libMesh::System * _source_system = nullptr;
  unsigned int _variable_number = 0;
  const unsigned int _num_nearest_points;
  std::vector<Sample> _samples;
  std::vector<Point> _points;
  // Declared last so the tree is destroyed before the points it references.
  std::unique_ptr<KDTree> _tree;
};

extern template class LibMeshNearestLocationEvaluator<Foam::scalar>;
