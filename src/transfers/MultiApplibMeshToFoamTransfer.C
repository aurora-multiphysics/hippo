#include "MooseEnum.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include "Registry.h"
#include "fvCFD_moose.h"
#include "InputParameters.h"
#include "FEProblemBase.h"
#include "FoamProblem.h"
#include "MultiApp.h"

#include "libmesh/mesh_function.h"
#include <memory>
#include <scalar.H>
#include <typeInfo.H>
#include <type_traits>
#include <vector.H>
#include <vectorField.H>
#include "MultiAppFoamTransfer.h"
#include "MultiApplibMeshToFoamTransfer.h"

registerMooseObject("hippoApp", MultiApplibMeshToFoamTransfer);

InputParameters
MultiApplibMeshToFoamTransfer::validParams()
{
  InputParameters params = MultiAppFoamTransfer::validParams();
  params.addParam<std::vector<std::string>>("foam_boundaries", "OpenFOAM boundaries.");
  params.addRequiredParam<VariableName>("foam_variable", "OpenFOAM variable");
  params.addRequiredParam<VariableName>("libmesh_variable", "OpenFOAM variable");
  return params;
}

MultiApplibMeshToFoamTransfer::MultiApplibMeshToFoamTransfer(const InputParameters & params)
  : MultiAppFoamTransfer(params),
    _mesh_function(),
    _foam_var_name(getParam<VariableName>("foam_variable")),
    _libmesh_var_name(getParam<VariableName>("libmesh_variable")),
    _outgoing_points()
{
}

template <typename T>
T
MultiApplibMeshToFoamTransfer::FoamOutOfRangeValue()
{
  if constexpr (std::is_same_v<T, Foam::scalar>)
    return Foam::VGREAT;
  else if constexpr (std::is_same_v<T, Foam::vector>)
    return Foam::vector(Foam::VGREAT, Foam::VGREAT, Foam::VGREAT);
  else
    mooseError("Invalid type for Foam transfer.");
}

template <typename T>
void
MultiApplibMeshToFoamTransfer::buildTransferMapping()
{
  _outgoing_points.clear();
  _n_points = 0;

  auto & foam_problem = foamProblem(getActiveToProblem());
  auto & foam_mesh = foam_problem.mesh().fvMesh();

  const auto & foam_boundaries = getParam<std::vector<std::string>>("foam_boundaries");
  if (foam_boundaries.empty())
  {
    const auto & cell_centers = foam_mesh.C();
    for (const auto & point : cell_centers)
    {
      libMesh::Point point_in_target_frame;
      for (auto d = 0; d < Foam::vector::nComponents; ++d)
      {
        point_in_target_frame(d) = point[d];
      }
      const auto point_in_source_frame = mapPointToActiveSourceFrame(point_in_target_frame);
      for (const auto i_proc : make_range(n_processors()))
        _outgoing_points[i_proc].push_back(point_in_source_frame);
    }

    _n_points = cell_centers.size();
  }
  else
  {
    for (const auto & boundary : foam_boundaries)
    {
      const auto & patch = foam_mesh.boundary()[boundary];
      const auto & cell_centers = patch.Cf();
      for (const auto & point : cell_centers)
      {
        libMesh::Point point_in_target_frame;
        for (auto d = 0; d < Foam::vector::nComponents; ++d)
        {
          point_in_target_frame(d) = point[d];
        }
        const auto point_in_source_frame = mapPointToActiveSourceFrame(point_in_target_frame);
        for (const auto i_proc : make_range(n_processors()))
          _outgoing_points[i_proc].push_back(point_in_source_frame);
      }
      _n_points += cell_centers.size();
    }
  }

  FEProblemBase & from_problem = getActiveFromProblem();
  MooseVariableFieldBase & from_var = from_problem.getVariable(
      0, _libmesh_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
  System & from_sys = from_var.sys().system();
  unsigned int from_var_num = from_sys.variable_number(_libmesh_var_name);
  // Construct a local mesh function for each origin problem
  _mesh_function = std::make_unique<libMesh::MeshFunction>(
      getlibMeshEquationSystem(from_problem, _displaced_source_mesh),
      *from_sys.current_local_solution,
      from_sys.get_dof_map(),
      from_var_num);
  _mesh_function->init();
  _mesh_function->enable_out_of_mesh_mode(FoamOutOfRangeValue<T>());
}

template <typename T>
void
MultiApplibMeshToFoamTransfer::transferVariables()
{
  // Evaluate interpolated values at incoming points.
  auto gather_functor = [&](processor_id_type /*pid*/,
                            const std::vector<Point> & incoming_points,
                            std::vector<std::pair<T, unsigned>> & vals_for_incoming_points)
  {
    vals_for_incoming_points.assign(incoming_points.size(), {0., 0});
    // Compute interpolation values of the libMesh variable at all requested points
    for (const auto i_pt : index_range(incoming_points))
    {
      const auto map = _mesh_function->discontinuous_value(incoming_points[i_pt]);
      for (const auto & [elem, val] : map)
        if (elem && elem->processor_id() == this->processor_id())
        {
          vals_for_incoming_points[i_pt].first += val;
          vals_for_incoming_points[i_pt].second++;
        }
    }
  };

  Foam::Field<T> interp_vals(_n_points);
  // Counters for the no. of elements sharing/neighboring a given point to perform arithmetic avg
  std::vector<unsigned> neighbor_elems(interp_vals.size(), 0);
  // Copy data out to interp_vals
  auto action_functor = [&](processor_id_type /*pid*/,
                            const std::vector<Point> & /*my_outgoing_points*/,
                            const std::vector<std::pair<T, unsigned>> & vals_for_outgoing_points)
  {
    for (const auto i : make_range(interp_vals.size()))
    {
      const auto val = vals_for_outgoing_points[i].first;
      const auto elems = vals_for_outgoing_points[i].second;

      if (!elems)
        continue;
      else if (interp_vals[i] == FoamOutOfRangeValue<T>())
        interp_vals[i] = val / (neighbor_elems[i] += elems);
      else
        interp_vals[i] = (interp_vals[i] * neighbor_elems[i] + val) / (neighbor_elems[i] += elems);
    }
  };

  // Set interpolated field values at points on local processor
  interp_vals = FoamOutOfRangeValue<T>(); // default to the out-of-mesh value
  // We assume incoming_vals is ordered in the same way as outgoing_points
  std::pair<T, unsigned> * dummy = nullptr;
  libMesh::Parallel::pull_parallel_vector_data(
      comm(), _outgoing_points, gather_functor, action_functor, dummy);
}

void
MultiApplibMeshToFoamTransfer::execute()
{
  getAppInfo();

  if (_current_direction != TO_MULTIAPP)
    mooseError("MultiApplibMeshToFoamTransfer currently supports only parent-to-child transfers.");

  auto multi_app = getToMultiApp();

  // TODO: Support all MultiApp instances and directions with per-pair mappings before merge.
  if (!multi_app->hasLocalApp(0))
    return;

  setActiveFromProblem(multi_app->problemBase(), 0);
  setActiveToProblem(multi_app->appProblemBase(0), 0);

  if (!_mapping_built)
  {
    buildTransferMapping<Foam::scalar>();
    _mapping_built = true;
  }

  transferVariables<Foam::scalar>();
}
