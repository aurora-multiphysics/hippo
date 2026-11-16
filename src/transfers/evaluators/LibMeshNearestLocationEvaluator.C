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

/// MOOSE constructor
template <typename T>
LibMeshNearestLocationEvaluator<T>::LibMeshNearestLocationEvaluator(
    FEProblemBase & problem,
    const VariableName & variable,
    bool use_displaced,
    const libMesh::Parallel::Communicator & communicator,
    const MooseEnum & source_type)
  : LibMeshNearestLocationEvaluator(getSourceVariable(problem, variable).sys().system(),
                                    getSourceVariable(problem, variable).number(),
                                    getSourceMesh(problem, use_displaced),
                                    communicator,
                                    source_type,
                                    getSourceVariable(problem, variable).isNodal(),
                                    problem.mesh().getMaxLeafSize())
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
    unsigned int max_leaf_size)
  : LibMeshTransferEvaluator<T>(communicator),
    _source_system(&system),
    _variable_number(variable_number)
{
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
      if (elem->p_level() > 0)
      {
        has_p_refinement = true;
        break;
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
      if (node->n_dofs(system_number, _variable_number))
      {
        _points.push_back(geometry_mesh.node_ref(node->id()));
        _samples.push_back(
            {node->id(), node->dof_number(system_number, _variable_number, 0), nullptr, Point()});
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
typename LibMeshNearestLocationEvaluator<T>::Candidate
LibMeshNearestLocationEvaluator<T>::evaluateLocal(const Point & point) const
{
  if (!_tree)
    return {};

  std::vector<std::size_t> indices;
  std::vector<Real> distances;
  const auto sample_count = static_cast<unsigned int>(_samples.size());
  unsigned int count = std::min(2u, sample_count);
  while (true)
  {
    _tree->neighborSearch(point, count, indices, distances);
    // Grow the query until every sample tied at the minimum distance is included.
    if (count == sample_count || distances.back() > distances.front())
      break;
    count = count > sample_count / 2 ? sample_count : count * 2;
  }
  // Queries are sorted by distance. Resolve all exact ties before reading a value.
  auto nearest_index = indices.front();
  for (std::size_t i = 1; i < indices.size() && distances[i] == distances.front(); ++i)
    if (_samples[indices[i]].entity_id < _samples[nearest_index].entity_id)
      nearest_index = indices[i];

  return sampleCandidate(nearest_index, distances.front());
}

template <typename T>
typename LibMeshNearestLocationEvaluator<T>::Candidate
LibMeshNearestLocationEvaluator<T>::sampleCandidate(std::size_t sample_index,
                                                    Real distance_squared) const
{
  const auto & sample = _samples[sample_index];
  Candidate result;
  result.count = 1;
  result.distance_squared = distance_squared;
  result.entity_id = sample.entity_id;
  // Both reads are local: do not synchronize the solution within this gather callback.
  if (sample.elem)
  {
    // Element-local FE evaluation includes the effective order from p-refinement.
    result.value = _source_system->point_value(_variable_number, sample.value_point, *sample.elem);
  }
  else
  {
    result.value = (*_source_system->current_local_solution)(sample.dof);
  }
  return result;
}

template <typename T>
void
LibMeshNearestLocationEvaluator<T>::accumulateContribution(Candidate & result,
                                                           const Candidate & candidate) const
{
  if (!candidate.count)
    return;
  if (!result.count || candidate.distance_squared < result.distance_squared ||
      (candidate.distance_squared == result.distance_squared &&
       candidate.entity_id < result.entity_id))
    result = candidate;
}

template class LibMeshNearestLocationEvaluator<Foam::scalar>;
