#include "LibMeshShapeFunctionEvaluator.h"
#include "FEProblemBase.h"
#include "DisplacedProblem.h"
#include "MooseError.h"
#include "MooseMesh.h"
#include "MooseVariableFieldBase.h"
#include "SystemBase.h"
#include "libmesh/elem.h"
#include "libmesh/system.h"

template <typename T>
LibMeshShapeFunctionEvaluator<T>::LibMeshShapeFunctionEvaluator(
    FEProblemBase & problem,
    const VariableName & variable,
    bool use_displaced,
    const libMesh::Parallel::Communicator & communicator)
  : LibMeshTransferEvaluator<T>(communicator),
    _source_processor_id(problem.mesh().getMesh().processor_id())
{
  auto & source_var = problem.getVariable(
      0, variable, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
  if (source_var.fieldType() != Moose::VarFieldType::VAR_FIELD_STANDARD)
    mooseError("The source must be a scalar field variable.");

  if (use_displaced && !problem.getDisplacedProblem())
    mooseError("No displaced problem to provide a displaced equation system");
  auto & equation_systems = use_displaced ? problem.getDisplacedProblem()->es() : problem.es();

  auto & source_system = source_var.sys().system();
  _mesh_function = std::make_unique<libMesh::MeshFunction>(equation_systems,
                                                           *source_system.current_local_solution,
                                                           source_system.get_dof_map(),
                                                           source_system.variable_number(variable));
  _mesh_function->init();
  // Permit misses on individual ranks; the distributed evaluator errors on global misses.
  _mesh_function->enable_out_of_mesh_mode(Foam::VGREAT);
}

template <typename T>
std::pair<T, unsigned>
LibMeshShapeFunctionEvaluator<T>::evaluateLocal(const Point & point) const
{
  std::pair<T, unsigned> result{0., 0};
  for (const auto & [elem, value] : _mesh_function->discontinuous_value(point))
    if (elem && elem->processor_id() == _source_processor_id)
    {
      result.first += value;
      ++result.second;
    }
  return result;
}

template class LibMeshShapeFunctionEvaluator<Foam::scalar>;
