#include "fvCFD_moose.h"
#include "LibMeshTransferEvaluator.h"
#include "MooseError.h"
#include "libmesh/parallel_algebra.h"
#include <map>

template <typename T>
std::vector<T>
LibMeshTransferEvaluator<T>::evaluate(const std::vector<Point> & points) const
{
  std::map<processor_id_type, std::vector<Point>> requests;
  if (!points.empty())
    for (processor_id_type pid = 0; pid < _communicator.size(); ++pid)
      requests.emplace(pid, points);

  std::vector<std::pair<T, unsigned>> results(points.size(), {T(0), 0});
  auto gather = [&](processor_id_type /*pid*/,
                    const std::vector<Point> & incoming_points,
                    std::vector<std::pair<T, unsigned>> & values)
  {
    values.resize(incoming_points.size());
    for (std::size_t i = 0; i < incoming_points.size(); ++i)
      values[i] = evaluateLocal(incoming_points[i]);
  };

  auto accumulate = [&](processor_id_type /*pid*/,
                        const std::vector<Point> & /*outgoing_points*/,
                        const std::vector<std::pair<T, unsigned>> & values)
  {
    for (std::size_t i = 0; i < values.size(); ++i)
    {
      results[i].first += values[i].first;
      results[i].second += values[i].second;
    }
  };

  std::pair<T, unsigned> * dummy = nullptr;
  libMesh::Parallel::pull_parallel_vector_data(_communicator, requests, gather, accumulate, dummy);

  // A local miss is valid until every source rank has answered. Synchronize failure
  // before returning so ranks with no missing points cannot start the next lookup.
  std::size_t missing_index = results.size();
  for (std::size_t i = 0; i < results.size(); ++i)
    if (!results[i].second)
    {
      missing_index = i;
      break;
    }
  bool missing = missing_index != results.size();
  _communicator.max(missing);
  if (missing)
  {
    if (missing_index != results.size())
      mooseError("No source element contributed to source-frame point ",
                 points[missing_index],
                 " (local request ",
                 missing_index,
                 ", rank ",
                 _communicator.rank(),
                 ").");
    mooseError("No source element contributed to a requested point on another rank.");
  }

  // Weight by contributing elements, not by ranks (which may own different counts).
  std::vector<T> values;
  values.reserve(results.size());
  for (const auto & [value, count] : results)
    values.push_back(value / count);

  return values;
}

template class LibMeshTransferEvaluator<Foam::scalar>;
