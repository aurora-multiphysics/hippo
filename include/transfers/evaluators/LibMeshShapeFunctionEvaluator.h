#pragma once

#include "fvCFD_moose.h"
#include "LibMeshTransferEvaluator.h"
#include "libmesh/mesh_function.h"
#include <memory>
#include <type_traits>

class FEProblemBase;

/// Evaluate scalar FE fields using locally owned elements, including discontinuous traces.
template <typename T>
class LibMeshShapeFunctionEvaluator : public LibMeshTransferEvaluator<T>
{
  static_assert(std::is_same_v<T, Foam::scalar>, "Only scalar FE evaluation is supported.");

public:
  /// The source problem and its solution must outlive this evaluator.
  LibMeshShapeFunctionEvaluator(FEProblemBase & problem,
                                const VariableName & variable,
                                bool use_displaced,
                                const libMesh::Parallel::Communicator & communicator);

protected:
  std::pair<T, unsigned> evaluateLocal(const Point & point) const override;

private:
  std::unique_ptr<libMesh::MeshFunction> _mesh_function;
  const processor_id_type _source_processor_id;
};

extern template class LibMeshShapeFunctionEvaluator<Foam::scalar>;
