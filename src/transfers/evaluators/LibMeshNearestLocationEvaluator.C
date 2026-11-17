#include "LibMeshNearestLocationEvaluator.h"
#include "FEProblemBase.h"
#include "MooseError.h"
#include "MooseEnum.h"
#include "MooseVariableFieldBase.h"
#include "SystemBase.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/fe_interface.h"
#include "libmesh/node.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/system.h"
#include "libmesh/variable.h"
#include <algorithm>
#include <limits>
#include <utility>

/// MOOSE constructor
template <typename T>
LibMeshNearestLocationEvaluator<T>::LibMeshNearestLocationEvaluator(
    FEProblemBase & problem,
    const VariableName & variable,
    bool use_displaced,
    const libMesh::Parallel::Communicator & communicator,
    const MooseEnum & source_type,
    unsigned int num_nearest_points)
  : LibMeshNearestLocationEvaluator(getSourceVariable(problem, variable).sys().system(),
                                    getSourceVariable(problem, variable).number(),
                                    getSourceMesh(problem, use_displaced),
                                    communicator,
                                    source_type,
                                    getSourceVariable(problem, variable).isNodal(),
                                    problem.mesh().getMaxLeafSize(),
                                    num_nearest_points)
{
}

/// Convenience constructor to allow testing independent of MOOSE
template <typename T>
LibMeshNearestLocationEvaluator<T>::LibMeshNearestLocationEvaluator(
    libMesh::System & system,
    unsigned int variable_number,
    libMesh::MeshBase & geometry_mesh,
    const libMesh::Parallel::Communicator & communicator,
    const MooseEnum & source_type,
    bool source_is_nodal,
    unsigned int max_leaf_size,
    unsigned int num_nearest_points)
  : LibMeshTransferEvaluator<T>(communicator),
    _source_system(&system),
    _variable_number(variable_number),
    _num_nearest_points(num_nearest_points)
{
  if (!_num_nearest_points)
    mooseError("num_nearest_points must be greater than zero.");

  const auto & fe_type = system.variable_type(variable_number);
  if (libMesh::FEInterface::field_type(fe_type) != libMesh::TYPE_SCALAR)
    mooseError("The source must be a scalar field variable.");

  auto & source_mesh = system.get_mesh();
  const auto system_number = _source_system->number();

  if (source_type != "nodes" && source_type != "centroids" && source_type != "variable_default")
    mooseError("Unknown nearest-location source_type: ", source_type, ".");
  const bool sample_nodes =
      source_type == "nodes" || (source_type == "variable_default" && source_is_nodal);

  if (sample_nodes)
  {
    bool has_p_refinement = false;
    for (const auto * elem : source_mesh.active_local_element_ptr_range())
    {
      if (elem->p_level() > 0)
      {
        has_p_refinement = true;
        break;
      }
    }
    communicator.max(has_p_refinement);
    if (has_p_refinement)
    {
      mooseError("Nearest-location node sampling does not support p-refined source meshes; "
                 "use source_type = centroids.");
    }

    if (libMesh::FEInterface::get_continuity(fe_type) == libMesh::DISCONTINUOUS ||
        libMesh::FEInterface::get_continuity(fe_type) == libMesh::SIDE_DISCONTINUOUS)
      mooseError("Source variable cannot be sampled at nodes as it is discontinuous.");

    if (fe_type.family != libMesh::LAGRANGE ||
        (fe_type.order != libMesh::FIRST && fe_type.order != libMesh::SECOND))
      mooseError(
          "Nearest-location node sampling supports only first- and second-order Lagrange fields.");

    for (const auto * node : source_mesh.local_node_ptr_range())
    {
      if (node->n_dofs(system_number, _variable_number))
      {
        _points.push_back(geometry_mesh.node_ref(node->id()));
        _samples.push_back(
            {node->id(), node->dof_number(system_number, _variable_number, 0), nullptr, Point()});
      }
    }
  }
  else
  {
    std::vector<dof_id_type> dofs;
    for (const auto * elem : source_mesh.active_local_element_ptr_range())
    {
      if (!_source_system->variable(_variable_number).active_on_subdomain(elem->subdomain_id()))
        continue;
      // Element DOFs alone exclude nodal fields, so inspect all variable DOFs on the element.
      _source_system->get_dof_map().dof_indices(elem, dofs, _variable_number);
      if (dofs.empty())
        continue;
      _points.push_back(geometry_mesh.elem_ref(elem->id()).vertex_average());
      _samples.push_back({elem->id(), 0, elem, elem->vertex_average()});
    }
  }

  // KDTree retains iterators into _points; never modify its storage after this point.
  if (!_points.empty())
  {
    if (_points.size() > std::numeric_limits<unsigned int>::max())
      mooseError("Too many local samples for the nearest-location KDTree query interface.");
    _tree = std::make_unique<KDTree>(_points, max_leaf_size);
  }
}

template <typename T>
std::vector<typename LibMeshNearestLocationEvaluator<T>::Candidate>
LibMeshNearestLocationEvaluator<T>::evaluateLocal(const Point & point) const
{
  if (!_tree)
    return {};

  const auto sample_count = static_cast<unsigned int>(_samples.size());
  const auto selected_count = std::min(_num_nearest_points, sample_count);
  auto search_count = selected_count;
  std::vector<std::size_t> indices;
  std::vector<Real> distances;
  while (true)
  {
    distances.resize(search_count);
    _tree->neighborSearch(point, search_count, indices, distances);

    // distance squared of Nth value (or number of samples)
    const Real cutoff_dist_squared = distances[selected_count - 1];
    // Whether the last sample is greater than the cutoff distance
    const bool found_farther_sample = distances.back() > cutoff_dist_squared;
    // Whether all samples have been searched
    const bool searched_all_samples = search_count == sample_count;
    // If either all samples have been searched or a sample found further away we are done
    if (searched_all_samples || found_farther_sample)
      break;

    // If not, double search count or select number of samples and repeat
    search_count = search_count > sample_count / 2 ? sample_count : search_count * 2;
  }

  // Keep squared distances and source sample indices while sorting.
  std::vector<std::pair<Real, std::size_t>> candidates;
  candidates.reserve(indices.size());
  for (std::size_t i = 0; i < indices.size(); ++i)
    candidates.emplace_back(distances[i], indices[i]);

  // Only the first selected_count candidates required
  std::partial_sort(candidates.begin(),
                    candidates.begin() + selected_count,
                    candidates.end(),
                    [this](const auto & a, const auto & b)
                    {
                      return a.first < b.first ||
                             (a.first == b.first &&
                              _samples[a.second].entity_id < _samples[b.second].entity_id);
                    });

  // Create vector with the selected candidates
  std::vector<Candidate> selected;
  selected.reserve(selected_count);
  for (unsigned int i = 0; i < selected_count; ++i)
  {
    const auto & [distance_squared, sample_index] = candidates[i];
    selected.push_back(
        {sampleValue(sample_index), 1, distance_squared, _samples[sample_index].entity_id});
  }
  return selected;
}

template <typename T>
T
LibMeshNearestLocationEvaluator<T>::sampleValue(std::size_t sample_index) const
{
  const auto & sample = _samples[sample_index];
  // Both reads are local: do not synchronize the solution within this gather callback.
  if (sample.elem)
  {
    // Element-local FE evaluation includes the effective order from p-refinement.
    return _source_system->point_value(_variable_number, sample.value_point, *sample.elem);
  }
  return (*_source_system->current_local_solution)(sample.dof);
}

template <typename T>
void
LibMeshNearestLocationEvaluator<T>::accumulateContribution(std::vector<Candidate> & results,
                                                           const Candidate & candidate) const
{
  if (!candidate.count)
    return;

  auto candidate_compare = [](const Candidate & a, const Candidate & b)
  {
    return a.distance_squared < b.distance_squared ||
           (a.distance_squared == b.distance_squared && a.entity_id < b.entity_id);
  };

  // find insertion position for given candidate
  const auto position =
      std::lower_bound(results.begin(), results.end(), candidate, candidate_compare);
  results.insert(position, candidate);

  // Remove candidates from back of vector if number of required points exceeded
  if (results.size() > _num_nearest_points)
    results.pop_back();
}

template class LibMeshNearestLocationEvaluator<Foam::scalar>;
