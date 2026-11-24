#include "LibMeshShapeFunctionEvaluator.h"
#include "FEProblemBase.h"
#include "MooseError.h"
#include "MooseMesh.h"
#include "MooseVariableFieldBase.h"
#include "SystemBase.h"
#include "libmesh/elem.h"
#include "libmesh/fe_interface.h"
#include "libmesh/system.h"
#include "libmesh/variable.h"

template <typename T>
LibMeshShapeFunctionEvaluator<T>::LibMeshShapeFunctionEvaluator(
    FEProblemBase & problem,
    const VariableName & variable,
    bool use_displaced,
    const libMesh::Parallel::Communicator & communicator)
  : LibMeshShapeFunctionEvaluator(getSourceVariable(problem, variable).sys().system(),
                                  getSourceVariable(problem, variable).number(),
                                  getSourceEquationSystems(problem, use_displaced),
                                  communicator)
{
}

template <typename T>
LibMeshShapeFunctionEvaluator<T>::LibMeshShapeFunctionEvaluator(
    libMesh::System & source_system,
    unsigned int variable_number,
    libMesh::EquationSystems & equation_systems,
    const libMesh::Parallel::Communicator & communicator)
  : LibMeshTransferEvaluator<T>(communicator),
    _source_processor_id(source_system.get_mesh().processor_id())
{
  if (libMesh::FEInterface::field_type(source_system.variable_type(variable_number)) !=
      libMesh::TYPE_SCALAR)
    mooseError("The source must be a scalar field variable.");
  _mesh_function = std::make_unique<libMesh::MeshFunction>(equation_systems,
                                                           *source_system.current_local_solution,
                                                           source_system.get_dof_map(),
                                                           variable_number);
  // An empty set means the variable is defined on every block.
  const auto & subdomains = source_system.variable(variable_number).active_subdomains();
  if (!subdomains.empty())
  {
    _mesh_function->set_subdomain_ids(&subdomains);
  }
  _mesh_function->init();
  // Permit misses on individual ranks; the distributed evaluator errors on global misses.
  _mesh_function->enable_out_of_mesh_mode(Foam::VGREAT);
}

template <typename T>
std::vector<typename LibMeshShapeFunctionEvaluator<T>::Candidate>
LibMeshShapeFunctionEvaluator<T>::evaluateLocal(const Point & point) const
{
  Candidate result;
  for (const auto & [elem, value] : _mesh_function->discontinuous_value(point))
    if (elem && elem->processor_id() == _source_processor_id)
    {
      result.value += value;
      ++result.count;
    }
  return {result};
}

template class LibMeshShapeFunctionEvaluator<Foam::scalar>;
