#pragma once

#include "fvCFD_moose.h"
#include "LibMeshTransferEvaluator.h"
#include "libmesh/mesh_function.h"
#include <memory>

class FEProblemBase;
namespace libMesh
{
class System;
}

/// Evaluate scalar FE fields using locally owned elements, including discontinuous traces.
template <typename T>
class LibMeshShapeFunctionEvaluator : public LibMeshTransferEvaluator<T>
{
public:
  /// The source problem and its solution must outlive this evaluator.
  LibMeshShapeFunctionEvaluator(FEProblemBase & problem,
                                const VariableName & variable,
                                bool use_displaced,
                                const libMesh::Parallel::Communicator & communicator);

  /// Direct libMesh entry point. All borrowed dependencies must outlive the evaluator.
  /// Geometry must have matching entity IDs, topology, and variable DOF numbering.
  LibMeshShapeFunctionEvaluator(libMesh::System & system,
                                unsigned int variable_number,
                                libMesh::EquationSystems & geometry,
                                const libMesh::Parallel::Communicator & communicator);

protected:
  using LibMeshTransferEvaluator<T>::getSourceVariable;
  using LibMeshTransferEvaluator<T>::getSourceEquationSystems;
  using Candidate = typename LibMeshTransferEvaluator<T>::Candidate;
  std::vector<Candidate> evaluateLocal(const Point & point) const override;

private:
  std::unique_ptr<libMesh::MeshFunction> _mesh_function;
  const processor_id_type _source_processor_id;
};

extern template class LibMeshShapeFunctionEvaluator<Foam::scalar>;
