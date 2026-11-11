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
#include "libmesh/parallel_algebra.h"
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
  params.addParam<std::vector<std::string>>(
      "foam_boundaries", std::vector<std::string>{}, "Native OpenFOAM destination patches.");
  params.addRequiredParam<VariableName>("foam_variable",
                                        "Registered OpenFOAM destination container name");
  params.addRequiredParam<VariableName>("libmesh_variable", "Source libMesh scalar field variable");
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
  static_assert(std::is_same_v<T, Foam::scalar>, "Only scalar boundary transfers are supported.");
  _outgoing_points.clear();

  auto & foam_problem = foamProblem(getActiveToProblem());
  const auto & patches = foam_problem.mesh().fvMesh().boundary();
  const auto & foam_boundaries = getParam<std::vector<std::string>>("foam_boundaries");

  for (const auto & boundary : foam_boundaries)
  {
    if (_outgoing_points.find(boundary) != _outgoing_points.end())
      mooseError("Patch '", boundary, "' is specified more than once.");

    const auto patch_id = patches.findIndex(boundary);
    if (patch_id < 0)
      mooseError("Unknown OpenFOAM patch '", boundary, "'.");

    const auto & patch = patches[patch_id];
    const auto & field = _boundary_transfer->get().boundaryField(boundary);
    if (field.size() != patch.size())
      mooseError("Destination field size does not match OpenFOAM patch '", boundary, "'.");

    auto & outgoing_points = _outgoing_points[boundary];
    for (const auto & point : patch.Cf())
    {
      libMesh::Point point_in_target_frame;
      for (auto d = 0; d < Foam::vector::nComponents; ++d)
        point_in_target_frame(d) = point[d];
      const auto point_in_source_frame = mapPointToActiveSourceFrame(point_in_target_frame);
      for (const auto i_proc : make_range(n_processors()))
        outgoing_points[i_proc].push_back(point_in_source_frame);
    }
  }
}

template <typename T>
void
MultiApplibMeshToFoamTransfer::prepareSourceEvaluator()
{
  FEProblemBase & from_problem = getActiveFromProblem();
  MooseVariableFieldBase & from_var = from_problem.getVariable(
      0, _libmesh_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
  if (from_var.fieldType() != Moose::VarFieldType::VAR_FIELD_STANDARD)
    mooseError("The source must be a scalar field variable.");
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
  const auto source_processor_id = getActiveFromProblem().mesh().getMesh().processor_id();
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
        if (elem && elem->processor_id() == source_processor_id)
        {
          vals_for_incoming_points[i_pt].first += val;
          vals_for_incoming_points[i_pt].second++;
        }
    }
  };

  static_assert(std::is_same_v<T, Foam::scalar>, "Only scalar boundary transfers are supported.");
  std::map<processor_id_type, std::vector<Point>> empty_requests;
  // All participating ranks process boundaries in input order, including locally empty patches.
  for (const auto & boundary : getParam<std::vector<std::string>>("foam_boundaries"))
  {
    std::vector<unsigned> neighbor_elems;
    if (_boundary_transfer)
      neighbor_elems.resize(_boundary_transfer->get().boundaryField(boundary).size(), 0);

    auto action_functor = [&](processor_id_type /*pid*/,
                              const std::vector<Point> & /*my_outgoing_points*/,
                              const std::vector<std::pair<T, unsigned>> & vals_for_outgoing_points)
    {
      if (!_boundary_transfer)
        return;

      auto & interp_vals = _boundary_transfer->get().boundaryField(boundary);
      for (const auto i : make_range(interp_vals.size()))
      {
        const auto val = vals_for_outgoing_points[i].first;
        const auto elems = vals_for_outgoing_points[i].second;
        if (!elems)
          continue;

        const auto previous_elems = neighbor_elems[i];
        neighbor_elems[i] += elems;
        if (!previous_elems)
          interp_vals[i] = val / neighbor_elems[i];
        else
          interp_vals[i] = (interp_vals[i] * previous_elems + val) / neighbor_elems[i];
      }
    };

    // Returned values follow the ordering of this boundary's requested face centres.
    std::pair<T, unsigned> * dummy = nullptr;
    auto & requests = _boundary_transfer ? _outgoing_points.at(boundary) : empty_requests;
    libMesh::Parallel::pull_parallel_vector_data(
        comm(), requests, gather_functor, action_functor, dummy);

    for (const auto i : index_range(neighbor_elems))
      if (!neighbor_elems[i])
        mooseError("No source element contributed to OpenFOAM boundary '",
                   boundary,
                   "', local face ",
                   i,
                   ".");
  }
}

void
MultiApplibMeshToFoamTransfer::execute()
{
  getAppInfo();

  if (_current_direction != TO_MULTIAPP)
    mooseError("MultiApplibMeshToFoamTransfer currently supports only parent-to-child transfers.");

  auto multi_app = getToMultiApp();

  // TODO: Support all MultiApp instances and directions with per-pair mappings before merge.
  setActiveFromProblem(multi_app->problemBase(), 0);
  const bool has_destination = multi_app->hasLocalApp(0);
  if (has_destination)
    setActiveToProblem(multi_app->appProblemBase(0), 0);

  if (!_mapping_built)
  {
    // Every parent rank can own source elements needed by a destination rank.
    prepareSourceEvaluator<Foam::scalar>();
    if (has_destination)
    {
      FoamProblem & foam_problem = foamProblem(getActiveToProblem());
      _boundary_transfer = std::ref(
          foam_problem.getBoundaryTransfer<Foam::scalar>(_foam_var_name, TransferQuantity::Value));
      buildTransferMapping<Foam::scalar>();
    }
    _mapping_built = true;
  }

  transferVariables<Foam::scalar>();
}
