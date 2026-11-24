#include "fvCFD_moose.h"
#include "LibMeshTransferEvaluator.h"
#include "MooseError.h"
#include "FEProblemBase.h"
#include "DisplacedProblem.h"
#include "MooseVariableFieldBase.h"
#include "libmesh/equation_systems.h"
#include "libmesh/parallel_algebra.h"
#include <map>

template <typename T>
MooseVariableFieldBase &
LibMeshTransferEvaluator<T>::getSourceVariable(FEProblemBase & problem,
                                               const VariableName & variable)
{
  auto & source = problem.getVariable(
      0, variable, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
  if (source.fieldType() != Moose::VarFieldType::VAR_FIELD_STANDARD)
    mooseError("The source must be a scalar field variable.");
  return source;
}

template <typename T>
libMesh::EquationSystems &
LibMeshTransferEvaluator<T>::getSourceEquationSystems(FEProblemBase & problem, bool use_displaced)
{
  if (use_displaced && !problem.getDisplacedProblem())
    mooseError("No displaced problem to provide displaced source geometry.");
  return use_displaced ? problem.getDisplacedProblem()->es() : problem.es();
}

template <typename T>
libMesh::MeshBase &
LibMeshTransferEvaluator<T>::getSourceMesh(FEProblemBase & problem, bool use_displaced)
{
  return getSourceEquationSystems(problem, use_displaced).get_mesh();
}

template <typename T>
void
LibMeshTransferEvaluator<T>::evaluate(const std::vector<Point> & points,
                                      Foam::Field<T> & destination) const
{
  bool size_mismatch = static_cast<std::size_t>(destination.size()) != points.size();
  _communicator.max(size_mismatch);
  if (size_mismatch)
    mooseError("Destination field size must match the evaluation point count on every rank.");

  std::map<processor_id_type, std::vector<Point>> requests;
  if (!points.empty())
    for (processor_id_type pid = 0; pid < _communicator.size(); ++pid)
      requests.emplace(pid, points);

  // TIMPI already supports tuples; keep their positional layout confined to communication.
  using Response = std::vector<typename Candidate::Tuple>;
  std::vector<std::vector<Candidate>> results(points.size());
  auto gather = [&](processor_id_type /*pid*/,
                    const std::vector<Point> & incoming_points,
                    std::vector<Response> & values)
  {
    values.resize(incoming_points.size());
    for (std::size_t i = 0; i < incoming_points.size(); ++i)
    {
      for (const auto & candidate : evaluateLocal(incoming_points[i]))
        if (candidate.count)
          values[i].push_back(candidate.asTuple());
    }
  };

  auto accumulate = [&](processor_id_type /*pid*/,
                        const std::vector<Point> & /*outgoing_points*/,
                        const std::vector<Response> & values)
  {
    for (std::size_t i = 0; i < values.size(); ++i)
      for (const auto & value : values[i])
        accumulateContribution(results[i], Candidate::fromTuple(value));
  };

  Response * dummy = nullptr;
  libMesh::Parallel::pull_parallel_vector_data(_communicator, requests, gather, accumulate, dummy);

  // A local miss is valid until every source rank has answered. Synchronize failure
  // before returning so ranks with no missing points cannot start the next lookup.
  std::size_t missing_index = results.size();
  for (std::size_t i = 0; i < results.size(); ++i)
    if (results[i].empty())
    {
      missing_index = i;
      break;
    }
  bool missing = missing_index != results.size();
  _communicator.max(missing);
  if (missing)
  {
    if (missing_index != results.size())
    {
      mooseError("No source sample contributed to source-frame point ",
                 points[missing_index],
                 " (local request ",
                 missing_index,
                 ", rank ",
                 _communicator.rank(),
                 ").");
    }
    mooseError("No source sample contributed to a requested point on another rank.");
  }

  for (std::size_t i = 0; i < results.size(); ++i)
  {
    T value = T(0);
    for (const auto & candidate : results[i])
      value += candidate.value;
    destination[i] = value / results[i].size();
  }
}

template <typename T>
void
LibMeshTransferEvaluator<T>::accumulateContribution(std::vector<Candidate> & results,
                                                    const Candidate & candidate) const
{
  if (!candidate.count)
    return;

  if (results.empty())
    results.emplace_back();
  auto & result = results.front();
  const auto total_count = result.count + candidate.count;
  result.value = (result.value * result.count + candidate.value) / total_count;
  result.count = total_count;
}

template class LibMeshTransferEvaluator<Foam::scalar>;
